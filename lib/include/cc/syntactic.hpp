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

#pragma once

#include <cc/dfa.hpp>
#include <cc/operation.hpp>
#include <cc/taint.hpp>
#include <cc/ir.hpp>

#include <optional>
#include <vector>

namespace lart
{
    using operation = lart::op::operation;

    template< typename T > using generator = cppcoro::generator< T >;

    struct syntactic : sc::with_context
    {
        explicit syntactic( llvm::Module &m, const dfa::types &t )
            : sc::with_context( m ), types( t ), module( m )
        {}

        std::optional< operation > make_operation( llvm::Value *value );

        generator< operation > toprocess();
        std::optional< ir::intrinsic > process( operation op );

        void propagate_identity( llvm::Value *from );

        void update_places( llvm::Value *concrete );

        std::map< llvm::Value*, llvm::Value* > abstract;
        std::map< llvm::Value*, llvm::Value* > identity;
        std::map< llvm::Value*, std::vector< ir::arg::liftable > > places;

        const dfa::types &types;
        llvm::Module &module;
    };

} // namespace lart
