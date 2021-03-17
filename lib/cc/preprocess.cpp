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

#include <cc/preprocess.hpp>

#include <cc/util.hpp>
#include <cc/logger.hpp>

#include <sc/ranges.hpp>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/Utils.h> // LowerSwitchPass

#include <algorithm>

namespace lart
{
    namespace sv = sc::views;

    void preprocessor::run( llvm::Function *fn )
    {
        if ( !util::tag_function_with_metadata( *fn, preprocessor::tag ) )
            return;

        spdlog::debug( "preprocess {}", fn->getName().str() );

        // TODO:
        // auto lse = std::unique_ptr< llvm::FunctionPass >( lart::createLowerSelectPass() );
        // lse.get()->runOnFunction( *fn );

        // TODO:
        // auto lsi = std::unique_ptr< llvm::FunctionPass >( llvm::createLowerSwitchPass() );
        // lsi.get()->runOnFunction( *fn );

        lower_cmps( fn );

        // TODO:
        // if ( duplicate_called_functions( fn ) )
        //    update_aa();
    }

    void preprocessor::lower_cmps( llvm::Function *fn )
    {
        auto lower = [fn] ( auto cmp ) {
            auto src = cmp->getParent();
            auto next = cmp->getNextNonDebugInstruction();
            auto dst = src->splitBasicBlock( next, "lart.lower.cmp" );

            auto irb = llvm::IRBuilder( cmp );
            src->getTerminator()->eraseFromParent();

            auto & ctx = cmp->getContext();
            auto tbb = llvm::BasicBlock::Create( ctx, "lart.lower.true", fn, dst );
            auto fbb = llvm::BasicBlock::Create( ctx, "lart.lower.false", fn, dst );

            irb.SetInsertPoint( tbb );
            irb.CreateBr( dst );

            irb.SetInsertPoint( fbb );
            irb.CreateBr( dst );

            irb.SetInsertPoint( dst, dst->getFirstInsertionPt() );
            auto phi = irb.CreatePHI( cmp->getType(), 2, "lowered" );
            phi->addIncoming( irb.getTrue(), tbb );
            phi->addIncoming( irb.getFalse(), fbb );

            cmp->replaceAllUsesWith( phi );

            auto br = irb.CreateCondBr( cmp, tbb, fbb );
            br->moveAfter( &cmp->getParent()->back() );
        };

        auto nonbr = [] ( auto i ) {
            return std::ranges::any_of( i->users(), sv::isnot< llvm::BranchInst > );
        };

        auto cmps = sv::filter< llvm::CmpInst >( *fn );
        for ( auto cmp : cmps | std::views::filter( nonbr ) )
            lower( cmp );
    }
}
