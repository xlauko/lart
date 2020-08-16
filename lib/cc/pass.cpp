/*
 * (c) 2020 Henrich Lauko <xlauko@mail.muni.cz>
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

#include "pass.hpp"

#include "stats.hpp"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

namespace lart
{
    bool pass::doInitialization( llvm::Module &m )
    {
        DEBUG_WITH_TYPE( "lart", dbgs() << "initialize lart\n" );
        _driver = std::make_unique< driver >( m );

        return true;
    }

    bool pass::runOnModule( llvm::Module & )
    {
        DEBUG_WITH_TYPE( "lart", dbgs() << "run lart\n" );
        return _driver->run();
    }

    void regiter_pass( const llvm::PassManagerBuilder &, llvm::legacy::PassManagerBase &pm )
    {
        pm.add( new lart::pass() );
    }

    char pass::ID = 0;

} // namespace lart

static llvm::RegisterPass< lart::pass > X( "lart", "Abstraction Pass" );
static struct llvm::RegisterStandardPasses Y( llvm::PassManagerBuilder::EP_VectorizerStart,
                                              lart::regiter_pass );
static struct llvm::RegisterStandardPasses Z( llvm::PassManagerBuilder::EP_EnabledOnOptLevel0,
                                              lart::regiter_pass );
