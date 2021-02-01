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

#include <cc/dfa.hpp>
#include <cc/syntactic.hpp>
#include <cc/logger.hpp>

#include <cc/backend/exec.hpp>

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

        // 3. syntactic pass
        syntactic syn( module, types );
        for ( const auto &op : syn.toprocess() )
            syn.process( op );

        /* Abstract IR */

        // 4. add assumes

        // 5. substitution

        // 6. release ?

        // 7. interrupts ?

        // TODO pick backend based on cmd arguments
        auto backend = lart::backend::exec();

        for ( auto call : sv::filter< llvm::CallInst >( module ) )
            if ( auto intr = backend.get_intrinsic( call ) )
                backend.lower( intr.value() );

        return true;
    }

} // namespace lart
