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

#include <cc/backend/native.hpp>
#include <cc/taint.hpp>

#include <sc/builder.hpp>

#include <cassert>

namespace lart::backend
{
    void native::lower( ir::intrinsic i, testtaint )
    {
        auto fn = i.call->getCalledFunction();
        if ( !fn->empty() )
            return; // already synthesized

        llvm::Value *tainted = sc::i1( false );

        auto bld = sc::stack_builder()
            | sc::action::function( fn )
            | sc::action::create_block( "entry" );

        auto test = [&] ( auto arg ) -> llvm::Value* {
            if ( arg->getType()->isIntegerTy() ) {
                bld = bld | sc::action::zfit( arg, sc::i8() )
                          | sc::action::call( testtaint_fn, { std::nullopt } );
                return bld.stack.back();
            }
            else if ( arg->getType()->isFloatingPointTy() ) {
                bld = bld | sc::action::fptoui( arg, sc::i8() )
                          | sc::action::call( testtaint_fn, { std::nullopt } );
                return bld.stack.back();
            }
            else if ( arg->getType()->isPointerTy() ) {
                bld = bld | sc::action::ptrtoint( arg, sc::i8() )
                          | sc::action::call( testtaint_fn, { std::nullopt } );
                return bld.stack.back();

            }
            return arg;
        };

        std::vector< llvm::Value* > args;
        // skip lifter argument
        unsigned pos = 1;

        for ( auto arg : op::arguments( i.op ) ) {
            if ( arg.type == op::argtype::lift ) {
                auto concrete = fn->getArg(pos);
                auto abstract = fn->getArg(pos + 1);
                auto taint = test( concrete );

                bld = bld | sc::action::or_( tainted, taint );
                tainted = bld.stack.back();

                args.push_back( taint );
                args.push_back( concrete );
                args.push_back( abstract );

                pos += 2;
            } else if ( arg.type == op::argtype::test ) {
                auto taint = test( fn->getArg(pos) );
                bld = bld | sc::action::or_( tainted, taint );
                tainted = bld.stack.back();
                pos++;
            } else {
                args.push_back( fn->getArg(pos) );
                pos++;
            }
        }

        bld = bld
            | sc::action::create_block( "abstract.path" )
            | sc::action::create_block( "concrete.path" )
            | sc::action::set_block( "entry" )
            | sc::action::condbr( tainted )
            /* abstract path */
            | sc::action::set_block( "abstract.path")
            | sc::action::call( fn->getArg(0), args )
            | sc::action::ret()
            /* concrete path */
            | sc::action::set_block( "concrete.path" );

        if ( auto def = op::default_value(i.op); def.has_value() ) {
            bld | sc::action::ret( extract_default( def.value(), args ) );
        } else {
            bld | sc::action::ret();
        }
    }

    void native::lower( callinst call, op::unstash )
    {
        call->setCalledFunction( unstash_fn );
    }

} // namespace lart::backend
