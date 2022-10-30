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
    void native::lower_test_taint( ir::intrinsic i )
    {
        spdlog::debug("[native] lower {}", sc::fmt::llvm_to_string(i.call));
        auto fn = i.call->getCalledFunction();
        if ( !fn->empty() )
            return; // already synthesized

        llvm::Value *tainted = sc::i1( false );

        auto bld = sc::stack_builder()
            | sc::action::function{ fn }
            | sc::action::create_block{ "entry" };

        std::vector< llvm::Value* > args;
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

        auto abstract_block = "abstract.path";
        auto concrete_block = "concrete.path";

        bld = std::move(bld)
            | sc::action::create_block{ abstract_block }
            | sc::action::create_block{ concrete_block };

        auto abb = bld.block( abstract_block );
        auto cbb = bld.block( concrete_block );

        auto fty = bld.function_type_from_value( fn->getArg(0) );
        auto callee = llvm::FunctionCallee( fty, fn->getArg(0) );

        bld = std::move(bld)
            | sc::action::set_block{ "entry" }
            | sc::action::condbr( tainted, abb, cbb )
            /* abstract path */
            | sc::action::set_block{ abstract_block }
            | sc::action::call( callee, args )
            | sc::action::ret()
            /* concrete path */
            | sc::action::set_block{ concrete_block };

        if ( auto def = op::default_value(i.op); def.has_value() ) {
            std::move(bld) | sc::action::ret{ extract_default( def.value(), args ) };
        } else {
            std::move(bld) | sc::action::ret();
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
