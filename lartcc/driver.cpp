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
#include <cc/shadow.hpp>
#include <cc/syntactic.hpp>
#include <cc/logger.hpp>
#include <cc/preprocess.hpp>
#include <cc/runtime.hpp>

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

        std::vector< ir::intrinsic > intrinsics;

        shadow_map shadows( module, types );
        syntactic syn( module, types, shadows );

        // generate unstash before shadow pass to use unstashed values as shadows
        for ( const auto &op : syn.unstash_toprocess() ) {
            if ( auto intr = syn.process( op ) ) {
                intrinsics.push_back( intr.value() );
            }
        }

        // generate shodows
        for (const auto &op : shadows.toprocess()) {
            shadows.process(op);
        }

        std::set< sc::function > seen;

        auto function = [&] (auto op) -> sc::function {
            if ( auto inst = llvm::dyn_cast< llvm::Instruction >( op::value( op ) ) ) {
                return sc::get_function(inst);
            }

            if ( auto arg = llvm::dyn_cast< llvm::Argument >( op::value( op ) ) ) {
                return sc::get_function(arg);
            }

            return nullptr;
        };

        // syntactic pass
        for ( const auto &op : syn.toprocess() ) {
            if (auto fn = function(op)) {
                seen.insert(fn);
            }

            if ( auto intr = syn.process( op ) ) {
                intrinsics.push_back( intr.value() );
            }
        }

        // 6. release ?
        // create/destroy frames

        // 7. interrupts ?

        // TODO pick backend based on cmd arguments
        auto backend = lart::backend::native( module );
        for ( auto intr : intrinsics ) {
            backend.lower( intr );
        }

        for ( auto intr : intrinsics ) {
            if (op::faultable(intr.op)) {
                llvm::cast< sc::instruction >(op::replaces(intr.op).value())->eraseFromParent();
            }
        }

        for (auto fn : seen) {
            make_shadow_frame(fn);
        }

        spdlog::info("lartcc finished");
        return llvm::PreservedAnalyses::none();
    }

} // namespace lart
