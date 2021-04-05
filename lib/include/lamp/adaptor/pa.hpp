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

#include <lava/pointer_arith.hpp>
#include <lava/term.hpp>
#include <lava/constant.hpp>

#include <lamp/support/semilattice.hpp>

namespace __lamp::adaptor
{
    using namespace __lava;

    template< typename arithmetic >
    struct pointers
    {
        using pa = pointer_arith< arithmetic >;

        using doms = domain_list< constant, arithmetic, pa >;
        using top = arithmetic;

        static constexpr auto pa_idx = doms::template idx< pa >;
        static constexpr auto ar_idx = doms::template idx< arithmetic >;
        static constexpr auto co_idx = doms::template idx< constant >;

        using scalar_lift_dom = constant;
        using scalar_any_dom  = arithmetic;
        using array_lift_dom  = constant;
        using array_any_dom   = arithmetic;

        static constexpr int join( int a, int b ) noexcept
        {
            if ( a == co_idx )
                return b;
            if ( b == co_idx )
                return a;
            if ( a == pa_idx || b == pa_idx )
                return pa_idx;
            return ar_idx;
        }

        static pa lift_objid( void* p ) { return pa::lift_objid( p ); }
    };

} // namespace __lamp::adaptor