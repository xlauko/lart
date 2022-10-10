/*
 * (c) 2022 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <cc/shadow.hpp>
#include <sc/builder.hpp>

namespace lart
{
    std::string to_string(shadow_op_kind kind) {
        switch (kind) {
            case shadow_op_kind::source:
                return "source";
            case shadow_op_kind::memory:
                return "memory";
            case shadow_op_kind::forward:
                return "forward";
            case shadow_op_kind::store:
                return "store";
            case shadow_op_kind::load:
                return "load";
            case shadow_op_kind::freeze:
                return "freeze";
            case shadow_op_kind::melt:
                return "melt";
            case shadow_op_kind::arg:
                return "argument";
            case shadow_op_kind::ret:
                return "return";
            case shadow_op_kind::global:
                return "global";
        }

        __builtin_unreachable();
    }

    std::optional< shadow_op_kind > shadow_map::operation_kind( sc::value val ) {
        if ( llvm::isa< llvm::AllocaInst >(val) ) {
            return shadow_op_kind::memory;
        } else if ( auto store = llvm::dyn_cast< llvm::StoreInst >(val) ) {
            auto p = store->getPointerOperand();
            auto v = store->getValueOperand();

            if ( types.count(p) ) {
                return shadow_op_kind::store;
            } else if ( types.count(v) && dfa::is_abstract( types[v] ) ) {
                return shadow_op_kind::freeze;
            } else {
                return std::nullopt;
            }
        } else if ( auto load = llvm::dyn_cast< llvm::LoadInst >(val) ) {
             auto p = load->getPointerOperand();
            if ( types.count(p) && dfa::is_abstract_pointer( types[p] ) ) {
                return shadow_op_kind::load;
            } else if ( types.count(load) && dfa::is_abstract( types[load] ) ) {
                return shadow_op_kind::melt;
            } else {
                return std::nullopt;
            }
        } else if ( auto call = llvm::dyn_cast< llvm::CallInst >(val) ) {
            auto fn = call->getCalledFunction();
            if ( fn->hasName() && fn->getName().startswith("__lamp" ) ) {
                return shadow_op_kind::source;
            }
        } else if ( llvm::isa< llvm::Argument >(val) ) {
            return shadow_op_kind::arg;
        } else if ( llvm::isa< llvm::GlobalVariable >(val) ) {
            return shadow_op_kind::global;
        }
        return shadow_op_kind::forward;
    }

    sc::generator< shadow_operation > shadow_map::toprocess()
    {
        for ( auto &[val, type] : types ) {
            if ( dfa::is_abstract( type ) ) {
                if ( auto kind = operation_kind(val) ) {
                    co_yield { val, kind.value() };
                }
            }
        }

        for ( auto store : sc::query::filter_llvm< llvm::StoreInst >( module ) ) {
            if ( auto kind = operation_kind(store) ) {
                co_yield { store, kind.value() };
            }
        }
    }

    sc::value shadow_map::process( shadow_operation o )
    {
        if (ops.count(o.value)) {
            return ops[o.value];
        }

        spdlog::debug("[shadow] make {} op: {}",
            to_string(o.kind),
            sc::fmt::llvm_to_string(o.value)
        );

        auto result = [&] {
            switch (o.kind) {
                case shadow_op_kind::source:
                    return process_source(o);
                case shadow_op_kind::memory:
                    return process_memory(o);
                case shadow_op_kind::forward:
                    return process_forward(o);
                case shadow_op_kind::store:
                    return process_store(o);
                case shadow_op_kind::load:
                    return process_load(o);
                case shadow_op_kind::freeze:
                    return process_freeze(o);
                case shadow_op_kind::melt:
                    return process_melt(o);
                case shadow_op_kind::arg:
                    return process_argument(o);
                case shadow_op_kind::ret:
                    return process_return(o);
                case shadow_op_kind::global:
                    return process_global(o);
            }

            __builtin_unreachable();
        } ();

        ops[o.value] = result;
        return result;
    }

    sc::value shadow_map::process( sc::value op ) {
        if (auto kind = operation_kind(op)) {
            return process({ op, kind.value() });
        }
        return sc::i1( false );
    }

    static sc::instruction* as_inst( sc::value value ) {
        return llvm::cast< sc::instruction >(value);
    }

    sc::value shadow_map::process_source( shadow_operation op ) {
        // is filled in syntactic pass
        return ops[op.value];
    }

    sc::value shadow_map::process_memory( shadow_operation op ) {
        return as_inst( sc::stack_builder( as_inst( op.value ) )
            | sc::action::alloc( sc::i1() )
            | sc::action::keep_stack()
            | sc::action::store{ sc::i1( false ), {} }
            | sc::action::last()
        );
    }

    sc::value shadow_map::process_forward( shadow_operation op ) {
        if (!llvm::isa< sc::instruction >( op.value )) {
            return sc::i1( false );
        }

        auto inst = as_inst( op.value );
        auto bld = sc::stack_builder( inst )
            | sc::action::push{ process(inst->getOperand(0)) };

        for (unsigned i = 1; i < inst->getNumOperands(); i++) {
            bld = std::move(bld) | sc::action::or_{ {}, process(inst->getOperand(i)) };
        }

        return bld.back();
    }

    sc::value shadow_map::process_store( shadow_operation /* op */ ) {
        // TODO
        return nullptr; // store does not return any value
    }

    sc::value shadow_map::process_load( shadow_operation /* op */ ) {
        // TODO
        return sc::i1( false );
    }

    sc::value shadow_map::process_melt( shadow_operation op ) {
        auto load = llvm::cast< llvm::LoadInst >( op.value );
        auto test = module.getFunction( "__lart_test_taint" );
        // TODO size?
        return sc::stack_builder( load )
            | sc::action::call( test, { load->getPointerOperand() } )
            | sc::action::last();
    }

    sc::value shadow_map::process_freeze( shadow_operation op ) {
        auto store = llvm::cast< llvm::StoreInst >( op.value );

        auto bld = sc::stack_builder( store );
        auto val = store->getValueOperand();
        auto ptr = store->getPointerOperand();

        std::move(bld) | sc::action::store{ process(val), process(ptr) };
        return nullptr; // store does not return any value
    }

    sc::value shadow_map::process_argument( shadow_operation /* op */ ) {
        // is filled in syntactic pass
        return sc::i1( false );
    }

    sc::value shadow_map::process_return( shadow_operation /* op */ ) {
        // is filled in syntactic pass
        return sc::i1( false );
    }

    sc::value shadow_map::process_global( shadow_operation op ) {
        auto glob = llvm::cast< llvm::GlobalVariable >( op.value );
        auto name = glob->getName() + ".lart.taint";
        return module.getOrInsertGlobal(name.str(), sc::i1(), [&] {
            return new llvm::GlobalVariable(
                module, sc::i1(), false, glob->getLinkage(), sc::i1(false), name
            );
        });
    }

    sc::value shadow_map::get( sc::value op ) const {
        if ( !ops.count(op) ) {
            return sc::i1(false);
        }
        return ops.at(op);
    }

    void make_shadow_frame( sc::function fn ) {
        auto entry_frame = fn->getParent()->getFunction( "__lart_entry_frame" );
        auto exit_frame = fn->getParent()->getFunction( "__lart_exit_frame" );

        sc::builder_t( fn->getEntryBlock().getFirstNonPHIOrDbgOrLifetime() ).CreateCall( entry_frame );

        for ( auto ret : sc::query::filter_llvm< llvm::ReturnInst >( fn ) ) {
            sc::builder_t( ret ).CreateCall( exit_frame );
        }
    }

} // namespace lart
