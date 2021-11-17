/*
 * (c) 2020, 2021 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <cc/driver.hpp>

#include <cc/assume.hpp>
#include <cc/dfa.hpp>
#include <cc/syntactic.hpp>
#include <cc/logger.hpp>
#include <cc/preprocess.hpp>

#include <cc/backend/native.hpp>

#include <llvm/Support/raw_os_ostream.h>
#include <llvm/IR/Verifier.h>

#include <sc/erase.hpp>
#include <sc/ranges.hpp>

#include <queue>
#include <vector>
#include <iostream>

namespace lart
{
    namespace sv = sc::views;

    void verify( llvm::Module &module )
    {
        llvm::raw_os_ostream cerr( std::cerr );
        if ( llvm::verifyModule( module, &cerr ) ) {
            cerr.flush(); // otherwise nothing will be displayed
            // module.print(llvm::errs(), nullptr);
            std::exit( EXIT_FAILURE );
        }
    }

    void replace_abstractable_functions( llvm::Module &module )
    {
        std::vector< std::pair< llvm::Function *, llvm::Function * > > abstractable;
        for (auto &fn : module) {
            auto name = fn.getName();
            std::string prefix = "__lamp_lifter_";
            if (name.startswith(prefix)) {
                auto suffix = name.drop_front(prefix.size());
                if (auto concrete = module.getFunction(suffix)) {
                    if (concrete->getType() == fn.getType()) {
                        abstractable.emplace_back( &fn, concrete );
                    } else {
                        spdlog::warn("mismatched type of abstractable function {}", suffix);
                    }
                }
            }
        }

        for ( auto [abs, con] : abstractable ) {
            for ( auto user : con->users() ) {
                user->replaceUsesOfWith( con, abs );
            }
        }
    }

    bool driver::run()
    {
        spdlog::cfg::load_env_levels();
        spdlog::info("lartcc started");

        preprocessor prep(module);
        for (auto &fn : module) {
            prep.run(fn);
        }

        replace_abstractable_functions(module);
        
        // propagate abstraction type from annotated roots
        auto types = dfa::analysis::run_on( module );

        // syntactic pass
        std::vector< ir::intrinsic > intrinsics;
        syntactic syn( module, types );
        for ( const auto &op : syn.toprocess() ) {
            if ( auto intr = syn.process( op ) ) {
                intrinsics.push_back( intr.value() );
            }
        }

        // 6. release ?

        // 7. interrupts ?

        // TODO pick backend based on cmd arguments
        auto backend = lart::backend::native( module );
        for ( auto intr : intrinsics ) {
            backend.lower( intr );
        }

        // delete stubs
        if ( auto stub = module.getFunction( "__lamp_stub" ) ) {
            stub->deleteBody();
        }

        spdlog::info("lartcc finished");
        // module.getFunction("main")->print(llvm::errs());        
        verify(module);
        return true;
    }

} // namespace lart
