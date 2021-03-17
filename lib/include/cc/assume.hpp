/*
 * (c) 2016 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/BasicBlock.h>

#include <cc/operation.hpp>

#include <external/coro/generator.hpp>

namespace lart::constrain
{
    struct assumption
    {
        assumption( llvm::Value *con, llvm::Constant *exp )
            : cond( con ), expect( exp )
        {}

        llvm::Value *cond;
        llvm::Constant *expect;
    };

    using basicblock = llvm::BasicBlock;

    struct bbedge
    {
        bbedge( basicblock *f, basicblock *t ) : from( f ), to( t ) {}

        void hide();
        void show();

        basicblock *from;
        basicblock *to;
    };

    struct assume_edge : bbedge
    {
        assume_edge( basicblock *f, basicblock *t ) : bbedge( f, t ) {}

        op::operation assume( assumption ass );
        unsigned succ_idx() const;
    };

    template< typename T > using generator = cppcoro::generator< T >;

    generator< op::operation > assume( llvm::BranchInst *br, llvm::Value *cond );

} // namespace lart::constrain
