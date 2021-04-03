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

#include <sc/ranges.hpp>

#include <ranges>
#include <queue>
#include <vector>
#include <iostream>

namespace lart
{
    namespace sv = sc::views;

    bool driver::run()
    {
        spdlog::cfg::load_env_levels();
        spdlog::info("start lartcc pass");

        // propagate abstraction type from annotated roots
        auto types = dfa::analysis::run_on( module );

        // lower pointer arithmetic to scalar operations
        for ( auto &[val, type] : types ) {
            if ( auto gep = llvm::dyn_cast< llvm::GetElementPtrInst >( val ) ) {
                for ( auto [src, inst] : lower_pointer_arithmetic( gep ) ) {
                    if ( types.count(src) )
                        types[inst] = types[src];
                }
                gep->eraseFromParent();
            }
        }

        // syntactic pass
        std::vector< ir::intrinsic > intrinsics;
        syntactic syn( module, types );
        for ( const auto &op : syn.toprocess() )
            intrinsics.push_back( syn.process( op ) );

        // 6. release ?

        // 7. interrupts ?

        // TODO pick backend based on cmd arguments
        auto backend = lart::backend::native( module );
        for ( auto intr : intrinsics )
            backend.lower( intr );

        // delete stubs
        for ( auto &fn : module ) {
            // TODO add stub annotation
            if ( fn.getName().startswith( "__lamp" ) )
                fn.deleteBody();

            if ( fn.getName().startswith( "__lart" ) )
                fn.deleteBody();
        }

        return true;
    }

} // namespace lart
