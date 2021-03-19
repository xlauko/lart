/*
 * (c) 2020 Henrich Lauko <xlauko@mail.muni.cz>
 * (c) 2020 Petr Ročkai <code@fixp.eu>
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

#include <lava/unit.hpp>
#include <lava/constant.hpp>

#include <lamp/support/semilattice.hpp>

namespace __lamp
{
    using namespace __lava;

    struct trivial
    {
        using doms = domain_list< unit, constant >;

        using scalar_lift_dom = constant;
        using scalar_any_dom = unit;
        using array_lift_dom = unit;
        using array_any_dom = unit;

        static constexpr int join( int a, int b ) noexcept
        {
            if ( a == doms::idx< constant > ) return b;
            if ( b == doms::idx< constant > ) return a;

            return doms::idx< unit >;
        }
    };

    using meta_domain = semilattice< trivial >;

} // namespace __lamp

#include "wrapper.hpp"
