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
#include <llvm/CodeGen/IntrinsicLowering.h>

#include <transforms/RemoveConstantExprs.hpp>
#include <transforms/LowerSwitch.hpp>

#include <algorithm>


namespace lart
{
    namespace sv = sc::views;

    void preprocessor::run( llvm::Function &fn )
    {
        if ( !util::tag_function_with_metadata( fn, preprocessor::tag ) )
            return;

        if ( fn.getName().startswith("__lamp") || fn.getName().startswith("__lava") )
            return;
        
        spdlog::debug( "preprocess {}", fn.getName().str() );
        bool change = false;
        
        do {
            change = false;
            // if ( lower_pointer_arithmetic( fn ) ) change = true;
            if ( lower_selects( fn ) ) change = true;
            if ( lower_constant_exprs( fn ) ) change = true;
            if ( lower_switch( fn ) ) change = true; 
            if ( lower_cmps( fn ) ) change = true;
            if ( lower_intrinsics( fn ) ) change = true;
        } while (change);

        auto fptobv = [] ( auto i ) {
            return i->getSrcTy()->isFloatingPointTy() && i->getDestTy()->isIntegerTy();
        };

        auto casts = sv::to_vector( sv::filter< llvm::BitCastInst >( fn ) );
        for ( auto cast : casts | std::views::filter( fptobv ) ) {
            auto irb = llvm::IRBuilder( cast );
            auto fpcast = irb.CreateFPToUI( cast->getOperand(0), cast->getDestTy() );
            cast->replaceAllUsesWith(fpcast);
            cast->eraseFromParent();
        }
    }


    bool preprocessor::lower_constant_exprs( llvm::Function &fn )
    {
        RemoveConstantExprs rce;
        return rce.runOnFunction( fn );
    }

    bool preprocessor::lower_selects( llvm::Function &fn )
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

        auto selects = sv::to_vector( sv::filter< llvm::SelectInst >( fn ) );
        for ( auto select : selects )
            lower( select );
        return !selects.empty();
    }

    bool preprocessor::lower_switch( llvm::Function &fn )
    {
        return llvm::LowerSwitch().runOnFunction( fn );
    }

    bool preprocessor::lower_cmps( llvm::Function &fn )
    {
        bool change = false;
        auto lower = [&] ( auto cmp ) {
            auto src = cmp->getParent();
            auto next = cmp->getNextNonDebugInstruction();
            auto dst = src->splitBasicBlock( next, "lart.lower.cmp" );

            auto irb = llvm::IRBuilder( cmp );
            src->getTerminator()->eraseFromParent();

            auto & ctx = cmp->getContext();
            auto tbb = llvm::BasicBlock::Create( ctx, "lart.lower.true", &fn, dst );
            auto fbb = llvm::BasicBlock::Create( ctx, "lart.lower.false", &fn, dst );

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
            change = true;
        };

        auto nonbr = [] ( auto i ) {
            return std::ranges::any_of( i->users(), sv::isnot< llvm::BranchInst > );
        };

        auto cmps = sv::filter< llvm::CmpInst >( fn );
        for ( auto cmp : cmps | std::views::filter( nonbr ) )
            lower( cmp );
        return change;
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

        void lower_to_arithmetic( llvm::GetElementPtrInst * gep, uint64_t accum_offset )
        {
            auto dl = llvm::DataLayout( sc::get_module( gep ) );

            llvm::IRBuilder irb( gep );
            auto pty = dl.getIntPtrType( gep->getType() );

            if ( llvm::isa< llvm::Constant >( gep->getOperand( 0 ) ) )
                return;
            
            auto res = instruction( irb.CreatePtrToInt( gep->getOperand( 0 ), pty ) );
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
                        } else {
                            val = irb.CreateMul( idx, constant( pty, element_size ) );
                        }
                        idx = val;
                    }

                    // Create an ADD for each index.
                    res = instruction( irb.CreateAdd( res, idx ) );
                }
            }

            // Create an ADD for the constant offset index.
            if ( accum_offset != 0 ) {
                res = instruction( irb.CreateAdd( res, constant( pty, accum_offset ) ) );
            }

            res = instruction( irb.CreateIntToPtr( res, gep->getType() ) );
            gep->replaceAllUsesWith( res );
        }
    } // anonymous namespace

    void lower_gep( llvm::GetElementPtrInst *gep )
    {
        canonicalize( gep );
        auto offset = accumulate_byte_offset( gep );
        lower_to_arithmetic( gep, offset );
    }

    bool preprocessor::lower_pointer_arithmetic( llvm::Function &fn )
    {
        std::vector< llvm::Instruction * > erase;
        
        auto geps = sv::filter< llvm::GetElementPtrInst >( fn );
        for ( auto gep : geps ) {
            lower_gep( gep );
            erase.push_back(gep);
        }

        bool change = !erase.empty();
        for (auto e : erase)
            e->eraseFromParent();
        return change;
    }


    template < class ArgIt >
    static llvm::CallInst *replace_call_with(const char *NewFn, llvm::CallInst *CI,
                                             ArgIt ArgBegin, ArgIt ArgEnd,
                                             llvm::Type *RetTy)
    {
        // If we haven't already looked up this function, check to see if the
        // program already contains a function with this name.
        llvm::Module *M = CI->getModule();
        // Get or insert the definition now.
        std::vector<llvm::Type *> ParamTys;
        for (ArgIt I = ArgBegin; I != ArgEnd; ++I)
            ParamTys.push_back((*I)->getType());
        auto FCache = M->getOrInsertFunction(NewFn, llvm::FunctionType::get(RetTy, ParamTys, false));

        llvm::IRBuilder<> Builder(CI->getParent(), CI->getIterator());
        llvm::SmallVector<llvm::Value *, 8> Args(ArgBegin, ArgEnd);
        llvm::CallInst *NewCI = Builder.CreateCall(FCache, Args);
        NewCI->setName(CI->getName());
        if (!CI->use_empty())
            CI->replaceAllUsesWith(NewCI);
        return NewCI;
    }

    static void replace_fpintrinsic_with_call(llvm::CallInst *CI, const char *Fname,
                                           const char *Dname,
                                           const char *LDname) 
    {
        switch (CI->getArgOperand(0)->getType()->getTypeID()) {
            default: llvm_unreachable("Invalid type in intrinsic");
            case llvm::Type::FloatTyID:
                replace_call_with(Fname, CI, CI->arg_begin(), CI->arg_end(),
                                llvm::Type::getFloatTy(CI->getContext()));
                break;
            case llvm::Type::DoubleTyID:
                replace_call_with(Dname, CI, CI->arg_begin(), CI->arg_end(),
                                llvm::Type::getDoubleTy(CI->getContext()));
                break;
            case llvm::Type::X86_FP80TyID:
            case llvm::Type::FP128TyID:
            case llvm::Type::PPC_FP128TyID:
                replace_call_with(LDname, CI, CI->arg_begin(), CI->arg_end(),
                                CI->getArgOperand(0)->getType());
                break;
        }
        CI->eraseFromParent();
    }

    bool preprocessor::lower_intrinsics( llvm::Function &fn )
    {
        bool change = false;

        auto keep = [] (auto id) -> bool {
            switch (id) {
                case llvm::Intrinsic::eh_typeid_for:
                case llvm::Intrinsic::trap:
                case llvm::Intrinsic::vastart:
                case llvm::Intrinsic::vacopy:
                case llvm::Intrinsic::vaend:
                case llvm::Intrinsic::stacksave:
                case llvm::Intrinsic::stackrestore:
                case llvm::Intrinsic::dbg_declare:
                case llvm::Intrinsic::dbg_value:
                case llvm::Intrinsic::lifetime_start:
                case llvm::Intrinsic::lifetime_end:
                    return true;
            }
            return false;
        };

        llvm::IntrinsicLowering il( module.getDataLayout() );

        auto intrinsics =  sv::to_vector( sv::filter< llvm::IntrinsicInst >( fn ) );
        for ( auto intr : intrinsics ) {
            auto id = intr->getIntrinsicID();
            if (keep(id)) continue;
            change = true;
            switch (id) {
                case llvm::Intrinsic::fabs:
                    replace_fpintrinsic_with_call( intr, "fabs", "fabsf", "fabsl" ); break;
                case llvm::Intrinsic::maxnum:
                    replace_fpintrinsic_with_call( intr, "fmaxf", "fmax", "fmaxl" ); break;
                case llvm::Intrinsic::minnum:
                    replace_fpintrinsic_with_call( intr, "fminf", "fmin", "fminl" ); break;
                case llvm::Intrinsic::rint:
                    replace_fpintrinsic_with_call( intr, "rintf", "rint", "rintl" ); break;
                case llvm::Intrinsic::nearbyint:
                    replace_fpintrinsic_with_call( intr, "nearbyintf", "nearbyint", "nearbyintl" ); break;
                case llvm::Intrinsic::round:
                    replace_fpintrinsic_with_call( intr, "roundf", "round", "roundl" ); break;
                default:
                    il.LowerIntrinsicCall( intr );
            }
        }

        return change;
    }

} // namespace lart