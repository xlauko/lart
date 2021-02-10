/*
 * (c) 2020 Petr Ročkai <code@fixp.eu>
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

/* This file is supposed to be included, but is not really a header. Instead,
 * it implements the LART-facing metadomain wrapper. Each metadomain should be
 * compiled into a separate bitcode file which contains exactly one copy
 * (build) of this file. See e.g. trivial.cpp for an example. */

#include <lamp/lamp.h>

#include <lava/support/base.hpp> /* iN */
#include <lava/constant.hpp>

using dom = __lamp::meta_domain;
using bw = __lava::bitwidth_t;

using namespace __lava; /* iN */

using ref = domain_ref< dom >;

struct wrapper
{
    template< typename op_t >
    __inline static __lamp_ptr wrap( op_t op ) { return { op().disown() }; }

    template< typename op_t, typename... args_t >
    __inline static __lamp_ptr wrap( op_t op, __lamp_ptr arg, args_t... args )
    {
        ref a( arg.ptr );
        return wrap( [&]( const auto & ... args_ ) __inline { return op( a, args_... ); }, args... );
    }

    template< typename op_t, typename arg_t, typename... args_t >
    __inline static auto wrap( op_t op, const arg_t &arg, args_t... args )
        -> std::enable_if_t< !std::is_same_v< arg_t, dom >, __lamp_ptr >
    {
        return wrap( [&] ( const auto & ... args_ ) __inline { return op( arg, args_... ); }, args... );
    }
};

template< typename... args_t >
__inline static auto wrap( const args_t & ... args ) { return wrapper::wrap( args... ); }
