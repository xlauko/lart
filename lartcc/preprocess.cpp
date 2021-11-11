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

#include "sc/ir.hpp"
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

        lower_selects( fn );

        auto lsi = std::unique_ptr< llvm::FunctionPass >( llvm::createLowerSwitchPass() );
        lsi.get()->runOnFunction( *fn );

        lower_cmps( fn );
    }

    void preprocessor::lower_selects( llvm::Function *fn )
    {
        auto lower = [] ( auto select ) {
            auto bb = select->getParent();
            // Split this basic block in half right before the select instruction.
            auto newCont = bb->splitBasicBlock( select, bb->getName()+".selectcont" );

            // Make the true block, and make it branch to the continue block.
            auto newTrue = llvm::BasicBlock::Create( bb->getContext(),
                        bb->getName()+".selecttrue", bb->getParent(), newCont );
            llvm::BranchInst::Create( newCont, newTrue );

            // Make the unconditional branch in the incoming block be a
            // conditional branch on the select predicate.
            bb->getInstList().erase( bb->getTerminator() );
            llvm::BranchInst::Create( newTrue, newCont, select->getCondition(), bb );

            // Create a new PHI node in the cont block with the entries we need.
            std::string name = select->getName().str(); select->setName("");
            auto *pn = llvm::PHINode::Create( select->getType(), 2, name, &*newCont->begin() );
            pn->addIncoming( select->getTrueValue(), newTrue );
            pn->addIncoming( select->getFalseValue(), bb );

            // Use the PHI instead of the select.
            select->replaceAllUsesWith( pn );
            newCont->getInstList().erase( select );
        };

        auto selects = sv::filter< llvm::SelectInst >( *fn );
        for ( auto select : selects )
            lower( select );
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

    namespace
    {
        void canonicalize( llvm::GetElementPtrInst *gep )
        {
            auto dl = llvm::DataLayout( sc::get_module( gep ) );

            auto *pty = dl.getIntPtrType( gep->getType() );
            auto gti = llvm::gep_type_begin( *gep );
            for ( auto *i = gep->op_begin() + 1; i != gep->op_end(); ++i, ++gti ) {
                // Skip struct member indices which must be i32.
                if ( gti.isSequential() )
                    if ( (*i)->getType() != pty )
                        *i = llvm::CastInst::CreateIntegerCast(*i, pty, true, "idxprom", gep );
            }
        }

        auto constant( llvm::Value *value ) { return llvm::cast< llvm::ConstantInt >( value ); }
        auto constant( llvm::Type *ty, auto v ) { return llvm::ConstantInt::get( ty, v ); }

        auto instruction( llvm::Value * val ) { return llvm::cast< llvm::Instruction >( val ); }

        uint64_t accumulate_byte_offset( llvm::GetElementPtrInst *gep )
        {
            auto dl = llvm::DataLayout( sc::get_module( gep ) );

            uint64_t offset = 0;
            auto gti = llvm::gep_type_begin( *gep );

            for ( unsigned i = 1; i != gep->getNumOperands(); ++i, ++gti ) {
                if ( !gti.isSequential() ) {
                    auto ty = gti.getStructType();
                    auto field = constant( gep->getOperand( i ) )->getZExtValue();
                    // Skip field 0 as the offset is always 0.
                    if ( field != 0 )
                        offset += dl.getStructLayout( ty )->getElementOffset( unsigned( field ) );
                }
            }

            return offset;
        }

        sc::generator< dependence > lower_to_arithmetic( llvm::GetElementPtrInst * gep, uint64_t accum_offset )
        {
            auto dl = llvm::DataLayout( sc::get_module( gep ) );

            llvm::IRBuilder irb( gep );
            auto pty = dl.getIntPtrType( gep->getType() );

            auto res = instruction( irb.CreatePtrToInt( gep->getOperand( 0 ), pty ) );
            co_yield { gep, res };
            auto gti = llvm::gep_type_begin( *gep );

            // Create ADD/SHL/MUL arithmetic operations for each sequential indices. We
            // don't create arithmetics for structure indices, as they are accumulated
            // in the constant offset index.
            for ( unsigned i = 1; i != gep->getNumOperands(); ++i, ++gti ) {
                if ( gti.isSequential() ) {
                    auto idx = gep->getOperand( i );
                    // Skip zero indices.
                    if ( auto ci = llvm::dyn_cast< llvm::ConstantInt >( idx ) )
                        if ( ci->isZero() )
                            continue;

                    auto element_size = llvm::APInt( pty->getIntegerBitWidth(),
                                                     dl.getTypeAllocSize( gti.getIndexedType() ) );

                    // Scale the index by element size.
                    if ( element_size != 1 ) {
                        llvm::Value * val = nullptr;
                        if ( element_size.isPowerOf2() ) {
                            val = irb.CreateShl( idx, constant( pty, element_size.logBase2() ) );
                            if ( auto ival = llvm::dyn_cast< llvm::Instruction >( val ) )
                                co_yield { idx, ival };
                        } else {
                            val = irb.CreateMul( idx, constant( pty, element_size ) );
                            if ( auto ival = llvm::dyn_cast< llvm::Instruction >( val ) )
                                co_yield { idx, ival };
                        }

                        idx = val;
                    }

                    // Create an ADD for each index.
                    res = instruction( irb.CreateAdd( res, idx ) );
                    co_yield { gep, res };
                }
            }

            // Create an ADD for the constant offset index.
            if ( accum_offset != 0 ) {
                res = instruction( irb.CreateAdd( res, constant( pty, accum_offset ) ) );
                co_yield { gep, res };
            }

            res = instruction( irb.CreateIntToPtr( res, gep->getType() ) );
            gep->replaceAllUsesWith( res );
            co_yield { gep, res };
        }
    } // anonymous namespace

    sc::generator< dependence > lower_pointer_arithmetic( llvm::GetElementPtrInst *gep )
    {
        canonicalize( gep );
        auto offset = accumulate_byte_offset( gep );
        for ( auto dep : lower_to_arithmetic( gep, offset ) )
            co_yield dep;
    }

} // namespace lart