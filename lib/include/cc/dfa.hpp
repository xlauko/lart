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

#include <cc/tristate.hpp>

#include <vector>

namespace lart
{
    struct type_layer
    {
        tristate pointer;
        tristate abstract;

        type_layer( bool p, bool a ) : pointer( p ), abstract( a ) {}
        type_layer( tristate p, tristate a ) : pointer( p ), abstract( a ) {}

        auto operator<=>( const type_layer & ) const = default;

        template< typename stream >
        friend auto operator<<( stream &s, type_layer t ) -> decltype( s << "" )
        {
            return s << "ptr:" << t.pointer
                    << " abs:" << t.abstract;
        }
    };

    inline type_layer join( type_layer a, type_layer b )
    {
        return { join( a.pointer, b.pointer ), join( a.abstract, b.abstract ) };
    }

    using type_vector = std::vector< type_layer >;

} // namespace lart
