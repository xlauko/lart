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
#include <cc/runtime.hpp>

#include <cc/backend/native/shadow.hpp>
#include <cc/backend/native/native.hpp>

#include <sc/erase.hpp>
#include <sc/query.hpp>

#include <queue>
#include <vector>
#include <iostream>

namespace lart
{
    llvm::PreservedAnalyses driver::run()
    {
        spdlog::cfg::load_env_levels();
        spdlog::info("lartcc started");

        runtime::initialize( module );

        // propagate abstraction type from annotated roots
        auto types = dfa::analysis::run_on( module );

        // lower pointer arithmetic to scalar operations
        {
            sc::deferred_erase_vector erase([&] (auto inst) { types.erase(inst); });

            for ( auto &[val, type] : types ) {
                if ( auto gep = llvm::dyn_cast< llvm::GetElementPtrInst >( val ) ) {
                    for ( auto [src, inst] : lower_pointer_arithmetic( gep ) ) {
                        if ( types.count(src) )
                            types[inst] = types[src];
                    }
                    erase.push(gep);
                }
            }
        }

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

        shadow instrument( module, types );
        for (const auto &op : instrument.toprocess()) {
            instrument.process(op);
        }

        // TODO pick backend based on cmd arguments
        auto backend = lart::backend::native( module );
        for ( auto intr : intrinsics ) {
            backend.lower( intr );
        }
        spdlog::info("lartcc finished");
        return llvm::PreservedAnalyses::none();
    }

} // namespace lart
