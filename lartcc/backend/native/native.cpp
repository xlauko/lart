/*
 * (c) 2021 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <cc/backend/native/native.hpp>
#include <cc/taint.hpp>

#include <sc/builder.hpp>

#include <cassert>

namespace lart::backend
{
    using function_callee = llvm::FunctionCallee;
    using args_t = std::vector< sc::value >;

    void native::lower_test_taint( ir::intrinsic i )
    {
        spdlog::debug("[native] lower {}", sc::fmt::llvm_to_string(i.call));
        auto fn = i.call->getCalledFunction();
        if ( !fn->empty() )
            return; // already synthesized

        sc::value tainted = sc::i1( false );

        auto bld = sc::stack_builder()
            | sc::action::function{ fn }
            | sc::action::create_block{ "entry" };

        args_t args;

        {
            // skip lifter argument
            unsigned pos = 1;

            for ( auto arg : op::arguments( i.op ) ) {
                switch (arg.type) {
                    case op::argtype::unpack:
                    case op::argtype::with_taint: {
                        auto taint    = fn->getArg(pos);
                        auto concrete = fn->getArg(pos + 1);
                        auto abstract = fn->getArg(pos + 2);

                        bld = std::move(bld) | sc::action::or_{ tainted, taint };
                        tainted = bld.stack.back();

                        args.push_back( taint );
                        args.push_back( concrete );
                        args.push_back( abstract );
                        pos += 3;
                        break;
                    }
                    case op::argtype::test: {
                        auto taint = fn->getArg(pos);
                        bld = std::move(bld) | sc::action::or_{ tainted, taint };
                        tainted = bld.stack.back();
                        pos += 1;
                        break;
                    }
                    case op::argtype::concrete: {
                        args.push_back( fn->getArg(pos) );
                        pos += 1;
                        break;
                    }
                    case op::argtype::abstract: {
                        // skip concrete argument
                        args.push_back( fn->getArg(pos + 1) );
                        pos += 2;
                        break;
                    }
                }
            }
        }

        auto abstract_block = "abstract.path";
        auto concrete_block = "concrete.path";

        bld = std::move(bld)
            | sc::action::create_block{ abstract_block }
            | sc::action::create_block{ concrete_block };

        auto abb = bld.block( abstract_block );
        auto cbb = bld.block( concrete_block );

        auto fty = bld.function_type_from_value( fn->getArg(0) );
        auto callee = function_callee( fty, fn->getArg(0) );

        bld = std::move(bld)
            | sc::action::set_block{ "entry" }
            | sc::action::condbr( tainted, abb, cbb )
            /* abstract path */
            | sc::action::set_block{ abstract_block }
            | sc::action::call( callee, args );

        auto lifter = bld.back();
        auto def = op::default_value(i.op);
        std::optional def_value = def.has_value()
            ? std::optional(extract_default( def.value(), args ))
            : std::nullopt;

        if (op::faultable(i.op)) {
            auto stash = function_callee(stash_fn);
            auto rty = fn->getReturnType();

            bld = std::move(bld)
                | sc::action::call( stash, { sc::i1(true), lifter } )
                | sc::action::ret{ llvm::Constant::getNullValue(rty) }
                /* concrete path */
                | sc::action::set_block{ concrete_block };

            auto inst = llvm::cast< sc::instruction >( op::value(i.op) )->clone();

            if (def_value.has_value()) {
                bld = std::move(bld) | sc::action::call( stash, { sc::i1(false), def_value } );

                inst->insertAfter( llvm::cast< sc::instruction >(bld.back()) );

                unsigned int idx = 0;
                unsigned int pos = 1;
                for ( auto &arg : op::arguments( i.op ) ) {
                    switch (arg.type) {
                        case op::argtype::unpack:
                        case op::argtype::with_taint: {
                            auto concrete = fn->getArg(pos + 1);
                            inst->setOperand(idx++, concrete);
                            pos += 3;
                            break;
                        }
                        case op::argtype::test: {
                            pos += 1;
                            break;
                        }
                        case op::argtype::concrete: {
                            auto concrete = fn->getArg(pos);
                            inst->setOperand(idx++, concrete);
                            pos += 1;
                            break;
                        }
                        case op::argtype::abstract: {
                            auto concrete = fn->getArg(pos);
                            inst->setOperand(idx++, concrete);
                            pos += 2;
                            break;
                        }
                    }
                }

                bld = std::move(bld) | sc::action::ret{ inst };
            } else {
                bld = std::move(bld) | sc::action::ret();
            }
        } else {
            bld = std::move(bld)
                | sc::action::ret()
                /* concrete path */
                | sc::action::set_block{ concrete_block };

            if (def_value.has_value()) {
                bld = std::move(bld) | sc::action::ret{ def_value };
            } else {
                bld = std::move(bld) | sc::action::ret();
            }
        }
    }

    void native::lower( callinst call, op::freeze )
    {
        auto abstract = call->getArgOperand( 2 );
        auto addr = call->getArgOperand( 3 );
        auto size = call->getArgOperand( 4 );
        auto bld = sc::builder_t( call );
        bld.call( freeze_fn, { abstract, addr, size } );
        call->eraseFromParent();
    }

    void native::lower( callinst call, op::unstash )
    {
        call->setCalledFunction( unstash_fn );
    }

    void native::lower( callinst call, op::unstash_taint )
    {
        call->setCalledFunction( unstash_taint_fn );
    }

    void native::lower( callinst call, op::dump op )
    {
        op.location()->eraseFromParent();
        call->setCalledFunction( dump_fn );
    }

    void native::lower( callinst call, op::stash )
    {
        auto taint    = call->getArgOperand( 0 );
        auto abstract = call->getArgOperand( 2 );
        auto bld = sc::builder_t( call );
        bld.call( stash_fn, { taint, abstract } );
        call->eraseFromParent();
    }

} // namespace lart::backend
