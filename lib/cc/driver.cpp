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

#include <cc/driver.hpp>

#include <svf/SVF-FE/LLVMUtil.h>
#include <svf/Graphs/SVFG.h>
#include <svf/WPA/Andersen.h>
#include <svf/SABER/LeakChecker.h>
#include <svf/SVF-FE/PAGBuilder.h>
#include <svf/SVF-FE/LLVMModule.h>

#include <sc/annotation.hpp>
#include <sc/case.hpp>

#include <ranges>
#include <queue>
#include <vector>
#include <iostream>

namespace lart
{
    std::set< llvm::CallSite > roots( llvm::Module &m )
    {
        auto ns = sc::annotation("lart", "abstract", "return");
        auto annotated = sc::annotation::enumerate_in_namespace< llvm::Function >(ns, m);

        std::queue< llvm::Value* > worklist;
        auto transitive_users = [&] (auto val) {
            for ( auto user : val->users() )
                worklist.emplace( user );
        };

        for ( auto fn : annotated | std::views::keys )
            transitive_users(fn);

        std::set< llvm::CallSite > result;
        while ( !worklist.empty() ) {
            sc::llvmcase( worklist.front(),
                [&] ( llvm::CastInst *c )     { transitive_users(c); },
                [&] ( llvm::ConstantExpr *c ) { transitive_users(c); },
                [&] ( llvm::CallInst *c )     { result.insert(c); },
                [&] ( llvm::InvokeInst * c)   { result.insert(c); }
            );
            worklist.pop();
        }
        return result;
    }

    bool driver::run()
    {
        // 1. process annotations
        for (auto root : roots(module)) {
            root->dump();
        }
        // 2. DFA

        // 3. syntactic pass ?

        /* Abstract IR */

        // 4. add assumes

        // 5. substitution

        // 6. release ?

        // 7. interrupts ?

        // 8. lart clean up
        return true;
    }

} // namespace lart
