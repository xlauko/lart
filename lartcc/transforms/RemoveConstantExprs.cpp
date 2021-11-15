#include <cassert>
#include <deque>

#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_os_ostream.h"
#include "sc/format.hpp"
#include <cc/logger.hpp>
#include <transforms/RemoveConstantExprs.hpp>

#include <sc/generator.hpp>

namespace lart
{
    sc::generator< llvm::Use* > constant_exprs(llvm::Instruction &inst) 
    {
        for (auto &op : inst.operands()) {
            if ( llvm::isa< llvm::ConstantExpr >( op.get() ) ) {
                co_yield &op;
            }
        }
    }

    bool RemoveConstantExprs::runOnFunction( llvm::Function &fn )
    {
        bool changed = false;

        std::deque< llvm::Use* > ces;

        auto enqueue = [&] (auto &inst) {
            for (auto ce : constant_exprs(inst)) {
                ces.push_back(ce);
            }
        };

        for (auto &inst : llvm::instructions(fn)) {
          enqueue(inst);
        }

        while (!ces.empty()) {
            auto use = ces.front();
            
            if ( auto *call = llvm::dyn_cast< llvm::CallInst >( use->getUser() ) ) {
                auto callee = call->getCalledOperand();
                if (callee == use->get()) {
                    ces.pop_front();
                    continue;
                }

                if (callee->hasName() && callee->getName().startswith("__lamp")) {
                    ces.pop_front();
                    continue;
                }
            }

            auto ce = llvm::cast< llvm::ConstantExpr >( use->get() );
            auto inst = ce->getAsInstruction();
            auto user = llvm::cast< llvm::Instruction >( use->getUser() );
            inst->insertBefore( user );
            use->set( inst );

            enqueue( *inst );

            ces.pop_front();
            changed = true;
        }

        return changed;
    }
} // namespace lart
