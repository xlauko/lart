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

#include <cc/backend/exec.hpp>

#include <sc/builder.hpp>

#include <cassert>

namespace lart::backend
{
    void exec::lower( ir::intrinsic i, testtaint )
    {
        auto fn = i.call->getCalledFunction();
        if ( !fn->empty() )
            return; // already synthesized

        //auto tainted = sc::i1( false );

        //std::vector< llvm::Value * > taints;

        // std::vector< llvm::Value * > args;
        // auto bld = sc::stack_builder()
        //     | sc::action::function( fn )
        //     | sc::action::create_block( "entry" );
    }

    void exec::lower( callinst call, op::unstash )
    {
        call->setCalledFunction( unstash_fn );
    }

} // namespace lart::backend
