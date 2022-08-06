/*
 * (c) 2022 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <cc/dfa.hpp>
#include <cc/operation.hpp>
#include <cc/ir.hpp>

#include <optional>
#include <vector>

namespace lart
{
    using operation = lart::op::operation;

    enum class shadow_op_kind {
        source, memory, forward, store, load
    };

    struct shadow_operation {
        llvm::Value *value;
        shadow_op_kind kind;
    };

    struct shadow : sc::with_context
    {
        explicit shadow( sc::module_ref &m, const dfa::types &t )
            : sc::with_context( m ), types( t ), module( m )
        {}

        sc::generator< shadow_operation > toprocess();

        std::optional< ir::intrinsic > process( shadow_operation op );

        std::optional< ir::intrinsic > process_source( shadow_operation op );

        std::optional< ir::intrinsic > process_memory( shadow_operation op );

        std::optional< ir::intrinsic > process_forward( shadow_operation op );

        std::optional< ir::intrinsic > process_load( shadow_operation op );

        std::optional< ir::intrinsic > process_store( shadow_operation op );

        const dfa::types &types;
        sc::module_ref module;
    };

} // namespace lart
