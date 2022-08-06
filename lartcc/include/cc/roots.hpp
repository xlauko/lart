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

#pragma once

#include <sc/annotation.hpp>
#include <sc/case.hpp>
#include <sc/builder.hpp>
#include <sc/types.hpp>

#include <cc/runtime.hpp>

#include <llvm/Support/ErrorHandling.h>

#include <map>
#include <queue>
#include <utility>

namespace lart
{
    enum class abstract_kind { scalar, pointer };

    using roots_map = std::map< llvm::CallBase*, abstract_kind >;

    inline std::optional< abstract_kind > get_abstract_kind(sc::function fn)
    {
        auto scalar = runtime::abstract_scalar_attr();
        if (fn->hasFnAttribute(scalar.getKindAsString()))
            return abstract_kind::scalar;

        auto pointer = runtime::abstract_pointer_attr();
        if (fn->hasFnAttribute(pointer.getKindAsString()))
            return abstract_kind::pointer;

        return std::nullopt;
    }

    using abstract_value_generator = std::pair< sc::function, abstract_kind >;
    inline sc::generator< abstract_value_generator > generators(sc::module_ref mod) {
        for (auto &fn : mod) {
            if (auto kind = get_abstract_kind(&fn)) {
                co_yield { &fn, kind.value() };
            }
        }
    }

    inline roots_map gather_roots(sc::module_ref m)
    {
        roots_map result;

        for (auto [gen, kind] : generators(m))
        {
            std::queue< llvm::Value* > worklist;
            auto transitive_users = [&] (auto val) {
                for ( auto user : val->users() ) {
                    worklist.emplace( user );
                }
            };

            auto register_root = [&] (auto root) {
                spdlog::debug("[dfa] root: {}", sc::fmt::llvm_to_string(root));
                result[root] = kind;
            };

            transitive_users(gen);
            while ( !worklist.empty() ) {
                sc::llvmcase( worklist.front(),
                    [&] ( llvm::CastInst *c )     { transitive_users(c); },
                    [&] ( llvm::ConstantExpr *c ) { transitive_users(c); },
                    [&] ( llvm::CallInst *c )     { register_root(c); },
                    [&] ( llvm::InvokeInst * c)   { register_root(c); }
                );
                worklist.pop();
            }
        }

        return result;
    }

} // namespace lart

