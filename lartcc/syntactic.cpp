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

#include <cc/arguments.hpp>
#include <cc/assume.hpp>
#include <cc/logger.hpp>
#include <cc/ir.hpp>
#include <cc/taint.hpp>

#include <sc/query.hpp>

#include <algorithm>

namespace lart
{
    namespace detail {
        bool is_call_with_prefix( const std::string &prefix, llvm::CallBase *call ) {
            if ( call->isIndirectCall() || !call->getCalledFunction() )
                return false;
            if ( !call->getCalledFunction()->hasName() )
                return false;
            return call->getCalledFunction()->getName().startswith( prefix );
        }
    } // namespace detail

    bool is_lamp_call( llvm::CallBase *call )
    {
        return detail::is_call_with_prefix( "__lamp_", call );
    }

    bool is_lart_call( llvm::CallBase *call )
    {
        return detail::is_call_with_prefix( "__lart_", call ) ||
               detail::is_call_with_prefix( "lart.", call );
    }

    bool is_dump_call( llvm::CallBase *call )
    {
        return detail::is_call_with_prefix( "__lamp_dump", call );
    }

    bool is_identity_cast( sc::value inst )
    {
        return util::is_one_of< llvm::BitCastInst
                              , llvm::PtrToIntInst
                              , llvm::IntToPtrInst >( inst );
    }

    std::optional< operation > syntactic::make_operation( sc::value val )
    {
        std::optional< operation > result;
        sc::llvmcase( val,
            [&] ( llvm::AllocaInst * ) {
                if ( dfa::is_abstract_pointer( types[val] ) )
                    result = op::alloc{ val };
            },
            [&] ( llvm::LoadInst *load ) {
                auto p = load->getPointerOperand();
                if ( types.count(p) && dfa::is_abstract_pointer( types[p] ) )
                    result = op::load{ val };
                else if ( dfa::is_abstract( types[load] ) )
                    result = op::melt{ val };
            },
            [&] ( llvm::StoreInst *store ) {
                auto abstract_value = [&] (auto v) {
                    return types.count(v) && dfa::is_abstract( types[v] );
                };

                auto abstract_content = [&] (auto v) {
                    return types.count(v) && dfa::is_abstract( types[v].peel() );
                };

                auto p = store->getPointerOperand();
                auto v = store->getValueOperand();

                if ( types.count(p) && dfa::is_abstract_pointer( types[p] ) ) {
                    result = op::store{ val };
                } else if (abstract_value(v) || abstract_content(p)) {
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
            [&] ( llvm::CallInst * ) {
                /* otherwise fallthrough, call unstash is processed behore shadow pass */
            },
            [&] ( llvm::Argument * ) {
                /* fallthrough */
            },
            [&] ( llvm::PHINode *phi ) {
                result = op::phi( phi );
            },
            [&] ( sc::value value ) {
                /* fallthrough */
                spdlog::warn( "ignore abstract: {}", sc::fmt::llvm_to_string(value) );
            }
        );
        return result;
    }

    sc::generator< operation > syntactic::unstash_toprocess()
    {
        auto is_abstract = [&] (auto value) {
            return types.count(value) && types[value].maybe_abstract();
        };

        auto function_has_abstract_arg = [&] (llvm::Function &fn) {
            for (auto &arg : fn.args())
                if (is_abstract(&arg))
                    return true;
            return false;
        };

        for ( auto &fn : module ) {
            if (function_has_abstract_arg(fn)) {
                for (auto &arg : fn.args()) {
                    auto point = fn.getEntryBlock().getFirstNonPHIOrDbg();
                    co_yield op::unstash(&arg, point);
                    co_yield op::unstash_taint(&arg, point);
                }
            }
        }

        for ( auto call : sc::query::filter_llvm< llvm::CallInst >( module ) ) {
            if ( is_abstract(call) ) {
                co_yield op::unstash(call);
                co_yield op::unstash_taint(call);
            }
        }

        for (auto o : toprocess()) {
            if ( op::returns_value(o) && op::faultable(o) ) {
                co_yield op::unstash(op::value(o));
                co_yield op::unstash_taint(op::value(o));
            }
        }
    }

    sc::generator< operation > syntactic::toprocess()
    {
        auto is_abstract = [&] (auto value) {
            return types.count(value) && types[value].maybe_abstract();
        };

        for ( const auto &[val, type] : types ) {
            if ( auto op = make_operation(val); op.has_value() ) {
                co_yield op.value();
            }
        }

        for ( auto store : sc::query::filter_llvm< llvm::StoreInst >( module ) ) {
            if ( is_abstract( store->getPointerOperand() ) )
                if ( auto op = make_operation(store); op.has_value() )
                    co_yield op.value();
        }

        for ( auto br : sc::query::filter_llvm< llvm::BranchInst >( module ) ) {
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

        for ( auto call : sc::query::filter_llvm< llvm::CallInst >( module ) ) {
            if ( !is_lart_call(call) && !is_lamp_call(call) ) {
                // TODO stash only possibly abstract arguments
                if ( has_abstract_arg(call) ) {
                    for (auto &arg : call->arg_operands())
                        co_yield op::stash(arg.get(), call);
                }
            }

            if ( is_dump_call(call) ) {
                co_yield op::dump( call->getArgOperand(0) );
            }
        }

        for ( auto ret : sc::query::filter_llvm< llvm::ReturnInst >( module ) ) {
            auto val = ret->getReturnValue();
            if ( is_abstract(val) )
                co_yield op::stash(val, ret);
        }
    }

    ir::intrinsic make_intrinsic( const lifter &lift )
    {
        auto op = lift.op;
        if ( op::emit_test_taint( op ) ) {
            return { taint::make_call( lift ), op };
        }

        std::vector< sc::value > args;
        for (auto arg : op::duplicated_arguments(op, lift.shadows)) {
            args.push_back(arg);
        }
        auto name = "lart." + op::name(op) + op::unique_name_suffix(op);
        return { op::make_call( op, args, name ), op };
    }

    void syntactic::propagate_identity( sc::value from )
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

    void syntactic::update_places( sc::value concrete )
    {
        auto abs = abstract[concrete];

        // Update placeholders where result of abstract operation
        // should be used instead. That are arguments of already
        // abstracted users of concrete variant of the operation.
        if ( auto args = places.find( concrete ); args != places.end() ) {
            for ( auto &place : args->second ) {
                std::visit( util::overloaded {
                    [&] (ir::arg::with_taint arg) {
                        arg.abstract.set( abs );
                    },
                    [&] (ir::arg::without_taint_abstract arg) {
                        arg.abstract.set( abs );
                    },
                    [] (ir::arg::without_taint_concrete /* arg */) {
                        spdlog::error("placing abstract value to concrete argument");
                    }
                }, place);
            }
            places.erase(args);
        }
    }

    std::optional< ir::intrinsic > syntactic::process( operation o )
    {
        spdlog::debug( "[syntactic] process {}", op::name(o) );

        if ( std::holds_alternative< op::identity >( o ) ) {
            auto cast = llvm::cast< llvm::CastInst >( op::value(o) );
            auto src = cast->getOperand( 0 );
            identity[src] = cast;
            propagate_identity(src);
            return std::nullopt;
        }

        // TODO join with taint processing
        if ( std::holds_alternative< op::phi >( o ) ) {
            // auto concrete = llvm::cast< llvm::PHINode >( op::value(o) );
            // llvm::IRBuilder<> irb( op::location(o) );
            // auto aptr = op::abstract_pointer();

            // auto phi = irb.CreatePHI( aptr->getType(), concrete->getNumIncomingValues() );
            // for ( unsigned i = 0; i < concrete->getNumIncomingValues(); ++i ) {
            //     phi->addIncoming( aptr, concrete->getIncomingBlock( i ) );
            // }

            // abstract[concrete] = phi;

            // for ( auto &arg : paired_view( concrete, phi ) ) {
            //     auto &con = arg.concrete;
            //     if ( auto abs = abstract.find( con.get() ); abs != abstract.end() )
            //         arg.abstract.set( abs->second );
            //     else
            //         places[con].push_back( arg );
            // }

            // update_places( concrete );
            // propagate_identity( concrete );

            return std::nullopt;
        }

        auto intr = make_intrinsic( lifter( module, o, shadows ) );

        if ( std::holds_alternative< op::unstash_taint >( o ) ) {
            shadows.ops[ op::value( o ) ] = intr.call;
        }

        // update places of abstract values, i.e., skip taint returning operations
        if ( op::returns_value(o) && !std::holds_alternative< op::unstash_taint >( o ) && !op::faultable(o) ) {
            auto concrete = op::value(o);
            abstract[concrete] = intr.call;
            update_places( concrete );
            propagate_identity( concrete );
        }

        // link faultable value with pregenerated unstash taints
        if ( op::returns_value(o) && op::faultable(o) ) {
            auto original_concrete = op::value(o);
            abstract[ intr.call ] = abstract[original_concrete];
            shadows.ops[ intr.call ] = shadows.ops[original_concrete];
        }

        if ( op::with_taints( intr.op ) || op::with_abstract_arg( intr.op ) ) {
            // Update placeholders of this instruction. If the abstract
            // instruction does not exist yet, store the placeholder
            // to quickly find it when abstract value is generated later.

            auto set_or_store_place = [&] (auto &con, auto &place, auto arg) {
                if ( auto abs = abstract.find( con.get() ); abs != abstract.end() ) {
                    place.set( abs->second );
                } else {
                    places[con].push_back( arg );
                }
            };

            for ( auto paired_arg : taint::paired_view( intr ) ) {
                std::visit( util::overloaded{
                    [&] (ir::arg::with_taint arg) { set_or_store_place(arg.concrete, arg.abstract, arg); },
                    [&] (ir::arg::without_taint_abstract arg) { set_or_store_place(arg.concrete, arg.abstract, arg); },
                    []  (ir::arg::without_taint_concrete /* arg */) { /* nothing to update */ }
                }, paired_arg);
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
