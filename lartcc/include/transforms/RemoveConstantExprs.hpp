#include <llvm/IR/Function.h>

namespace lart
{
  struct RemoveConstantExprs {
    bool runOnFunction( llvm::Function &F );
  };
} // namespace lart

