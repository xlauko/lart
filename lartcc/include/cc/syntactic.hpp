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

#include <cc/shadow.hpp>

#include <optional>
#include <vector>

namespace lart
{
    using operation = lart::op::operation;

    struct syntactic : sc::with_context
    {
        explicit syntactic( sc::module_ref m, const dfa::types &t, shadow_map &s )
            : sc::with_context( m ), types( t ), shadows( s ), module( m )
        {}

        std::optional< operation > make_operation( sc::value value );

        sc::generator< operation > unstash_toprocess();
        sc::generator< operation > toprocess();
        std::optional< ir::intrinsic > process( operation op );

        void propagate_identity( sc::value from );

        void update_places( sc::value concrete );

        std::map< sc::value, sc::value > abstract;
        std::map< sc::value, sc::value > identity;
        std::map< sc::value, std::vector< ir::argument > > places;

        const dfa::types &types;
        shadow_map &shadows;
        sc::module_ref module;
    };

} // namespace lart
