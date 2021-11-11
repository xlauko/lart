/*
 * (c) 2020, 2021 Henrich Lauko <xlauko@mail.muni.cz>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <cc/syntactic.hpp>

#include <cc/assume.hpp>
#include <cc/logger.hpp>
#include <cc/ir.hpp>
#include <cc/taint.hpp>

#include <sc/ranges.hpp>

#include <algorithm>

namespace lart
{
    namespace sv = sc::views;

    bool is_abstract( const dfa::types::type &type )
    {
        return static_cast< bool >( type.back().abstract );
    }

    bool is_abstract_pointer( const dfa::types::type &type )
    {
        return static_cast< bool >( type.back().pointer ) && is_abstract( type );
    }

    bool is_lamp_call( llvm::CallBase *call )
    {
        if ( call->isIndirectCall() )
            return false;
        return call->getCalledFunction()->getName().startswith( "__lamp" );
    }

    bool is_testtaint( llvm::CallBase *call )
    {
        if ( call->isIndirectCall() )
            return false;
        return call->getCalledFunction()->getName().startswith( "lart.test.taint" );
    }

    bool is_identity_cast( llvm::Value *inst )
    {
        return util::is_one_of< llvm::BitCastInst
                              , llvm::PtrToIntInst
                              , llvm::IntToPtrInst >( inst );
    }

    std::optional< operation > syntactic::make_operation( llvm::Value *val )
    {
        std::optional< operation > result;
        sc::llvmcase( val,
            [&] ( llvm::AllocaInst * ) {
                if ( is_abstract_pointer( types[val] ) )
                    result = op::alloc{ val };
            },
            [&] ( llvm::LoadInst *load ) {
                auto p = load->getPointerOperand();
                if ( types.count(p) && is_abstract_pointer( types[p] ) )
                    result = op::load{ val };
                else
                    result = op::melt{ val };
            },
            [&] ( llvm::StoreInst *store ) {
                auto p = store->getPointerOperand();
                auto v = store->getValueOperand();

                if ( types.count(p) && is_abstract_pointer( types[p] ) ) {
                    result = op::store{ val };
                } else if ( types.count(v) &&  is_abstract( types[v] ) ) {
                    result = op::freeze{ val };
                }
            },
            [&] ( llvm::BinaryOperator * ) {
                result = op::binary{ val };
            },
            [&] ( llvm::CmpInst * ) {
                result = op::cmp{ val };
            },
            [&] ( llvm::CastInst * ) {
                if ( is_identity_cast( val ) )
                    result = op::identity{ val };
                else
                    result = op::cast{ val };
            },
            [&] ( llvm::CallInst *call ) {
                if ( is_lamp_call(call) ) {
                    // TODO does return nonvoid
                    result = op::unstash{ call };
                } else {
                    // TODO
                }
            },
            [&] ( llvm::Argument * ) { /* fallthrough */ },
            [&] ( llvm::Value *value ) { 
                /* fallthrough */
                std::string buff;
                llvm::raw_string_ostream ss(buff);
                value->print(ss);
                spdlog::warn( "ignore abstract: {}", ss.str() );
            }
        );
        return result;
    }

    sc::generator< operation > syntactic::toprocess()
    {
        // TODO: rangeify
        auto is_abstract = [&] (auto value) {
            return types.count(value) && types[value].maybe_abstract();
        };

        for ( const auto &[val, type] : types ) {
            if ( auto op = make_operation(val); op.has_value() ) {
                co_yield op.value();
            }
        }

        for ( auto store : sv::filter< llvm::StoreInst >( module ) ) {
            if ( is_abstract( store->getPointerOperand() ) )
                if ( auto op = make_operation(store); op.has_value() )
                    co_yield op.value();
        }

        for ( auto br : sv::filter< llvm::BranchInst >( module ) ) {
            if ( !br->isConditional() )
                continue;
            auto cond = br->getCondition();
            if ( is_abstract(cond) )
            {
                co_yield op::tobool( br );

                for ( auto intr : constrain::assume( br, cond ) )
                    co_yield intr;
            }
        }

        auto has_abstract_arg = [&] (llvm::CallBase *call) {
            for (const auto &arg : call->arg_operands())
                if (is_abstract(arg.get()))
                    return true;
            return false;
        };

        for ( auto call : sv::filter< llvm::CallInst >( module ) ) {
            if ( !is_testtaint(call) && !is_lamp_call(call) ) {
                // TODO stash only possibly abstract arguments
                if ( has_abstract_arg(call) ) {
                    for (auto &arg : call->arg_operands())
                        co_yield op::stash(arg.get(), call);
                }

                if ( is_abstract(call) )
                    co_yield op::unstash(call);
            }
        }

        for ( auto ret : sv::filter< llvm::ReturnInst >( module ) ) {
            auto val = ret->getReturnValue();
            if ( is_abstract(val) )
                co_yield op::stash(val, ret);
        }

        auto function_has_abstract_arg = [&] (llvm::Function &fn) {
            for (auto &arg : fn.args())
                if (is_abstract(&arg))
                    return true;
            return false;
        };

        for ( auto &fn : module ) {
            if (function_has_abstract_arg(fn)) {
                for (auto &arg : fn.args())
                    co_yield op::unstash(&arg, fn.getEntryBlock().getFirstNonPHIOrDbg() );
            }
        }
    }

    ir::intrinsic make_intrinsic( llvm::Module &m, const operation &op )
    {
        if ( op::with_taints( op ) )
            return { taint::make_call( m, op ), op };

        auto args = sc::views::to_vector( op::duplicated_arguments(op) );
        auto name = "lart." + op::name(op);
        return { op::make_call( op, args, name ), op };
    }

    void syntactic::propagate_identity( llvm::Value *from )
    {
        if ( auto id = identity.find( from ); id != identity.end() ) {
            auto &[src, dst] = *id;
            assert( !abstract.count(dst) );
            if ( abstract.count(src) ) {
                abstract[dst] = abstract[src];
                update_places( dst );
                propagate_identity( dst );
            }
        }
    }

    void syntactic::update_places( llvm::Value *concrete )
    {
        auto abs = abstract[concrete];

        // Update placeholders where result of abstract operation
        // should be used instead. That are arguments of already
        // abstracted users of concrete variant of the operation.
        if ( auto args = places.find( concrete ); args != places.end() ) {
            for ( auto &place : args->second )
                place.abstract.set( abs );
            places.erase(args);
        }
    }

    std::optional< ir::intrinsic > syntactic::process( operation o )
    {
        spdlog::debug( "process {}", op::name(o) );

        if ( std::holds_alternative< op::identity >( o ) ) {
            auto cast = llvm::cast< llvm::CastInst >( op::value(o) );
            auto src = cast->getOperand( 0 );
            identity[src] = cast;
            propagate_identity(src);
            return std::nullopt;
        }

        auto intr = make_intrinsic( module, o );

        if ( op::returns_value(o) ) {
            auto concrete = op::value(o);
            abstract[concrete] = intr.call;

            update_places( concrete );
            propagate_identity( concrete );
        }

        if ( op::with_taints( intr.op ) || op::with_abstract_arg( intr.op ) ) {
            // Update placeholders of this instruction. If the abstract
            // instruction does not exist yet, store the placeholder
            // to quickly find it when abstract value is generated later.
            for ( auto arg : taint::paired_view( intr ) ) {
                auto &con = arg.concrete;
                if ( auto abs = abstract.find( con.get() ); abs != abstract.end() )
                    arg.abstract.set( abs->second );
                else
                    places[con].push_back( arg );
            }
        }

        // use result of intrinsic instead of concrete value
        // in default case (when concrete path was taken),
        // intrinsic returns the concrete value
        if ( auto re = op::replaces(intr.op) ) {
            for ( auto &u : re.value()->uses() ) {
                if ( u.getUser() != intr.call ) {
                    u.set( intr.call );
                }
            }
        }

        return intr;
    }

} // namespace lart
