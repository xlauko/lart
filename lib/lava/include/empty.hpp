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

#include <lava/support/tristate.hpp>
#include <lava/support/base.hpp>
#include <lava/constant.hpp>

namespace __lava
{
    struct empty_storage {};

    /* Minimal domain example. */
    template< template< typename > typename storage >
    struct empty : storage< empty_storage >
                 , domain_mixin< empty< storage > >
    {
        using base = storage< empty_storage >;
        using mixin = domain_mixin< unit >;
        using bw = typename mixin::bw;

        using base::base;

        using ev = empty;
        using er = const empty &;

        template< typename type > static ev lift( const type& ) { return {}; }
        template< typename type > static ev any() { return {}; }

        static void assume( er, bool ) {}

        static tristate to_tristate( er ) { return maybe; }
    };

} // namespace __lava
