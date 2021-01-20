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

namespace lart
{
    struct tristate
    {
        enum { no, yes, maybe } value;
        explicit tristate( bool v ) : value( v ? yes : no ) {}
        tristate( decltype( value ) v ) : value( v ) {}

        auto operator<=>( const tristate & ) const = default;

        operator bool() const
        {
            return value != no;
        }

        template< typename stream >
        friend auto operator<<( stream &s, tristate t ) -> decltype( s << "" )
        {
            switch ( t.value ) {
                case no: return s << "no";
                case yes: return s << "yes";
                case maybe: return s << "maybe";
                default: __builtin_unreachable();
            }
        }
    };

    inline tristate join( tristate a, tristate b )
    {
        if ( a.value == b.value )
            return a;
        return tristate( tristate::maybe );
    }

} // namespace lart
