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

#include <lava/support/tristate.hpp>
#include <lava/support/base.hpp>
#include <runtime/lart.h>

#pragma once

namespace __lava
{
    template< typename dom, bool _signed = false, bool floating = false >
    struct scalar_domain_ref : domain_ref< dom >
    {
        static_assert( !(floating && !_signed) && "unsigned floats are not permitted" );

        using base = domain_ref< dom >;

        using base::base;
        using sref = scalar_domain_ref;

        dom& self() { return *reinterpret_cast< dom* >( this ); }
        const dom& self() const { return *reinterpret_cast< dom* >( this ); }

        operator dom&() { return self(); }
        operator const dom&() const { return self(); }

        explicit operator bool()
        {
            auto v = dom::to_tristate( self() );

            switch ( v.value )
            {
                case tristate::false_value:  return false;
                case tristate::true_value: return true;
                case tristate::maybe_value:
                    bool rv = __lart_choose( 2 );
                    dom::assume( self(), rv );
                    return rv;
            }
        }

        friend sref concat( sref a, sref b ) { return { dom::op_concat( a.self(), b.self() ) }; }

        static constexpr auto bv_div = _signed ? dom::op_sdiv : dom::op_udiv;
        static constexpr auto bv_rem = _signed ? dom::op_srem : dom::op_urem;

        static constexpr auto ge = _signed ? dom::op_sge : dom::op_uge;
        static constexpr auto le = _signed ? dom::op_sle : dom::op_ule;
        static constexpr auto lt = _signed ? dom::op_slt : dom::op_ult;
        static constexpr auto gt = _signed ? dom::op_sgt : dom::op_ugt;

        static constexpr auto add = floating ? dom::op_fadd : dom::op_add;
        static constexpr auto sub = floating ? dom::op_fsub : dom::op_sub;
        static constexpr auto mul = floating ? dom::op_fmul : dom::op_mul;
        static constexpr auto div = floating ? dom::op_fdiv : bv_div;
        static constexpr auto rem = floating ? dom::op_frem : bv_rem;

        static constexpr auto shr = _signed ? dom::op_ashr : dom::op_lshr;

        friend sref operator+( sref a, sref b ) { return add( a.self(), b.self() ); }
        friend sref operator-( sref a, sref b ) { return sub( a.self(), b.self() ); }
        friend sref operator*( sref a, sref b ) { return mul( a.self(), b.self() ); }
        friend sref operator/( sref a, sref b ) { return div( a.self(), b.self() ); }
        friend sref operator%( sref a, sref b ) { return rem( a.self(), b.self() ); }

        friend sref operator&( sref a, sref b ) { return dom::op_and( a.self(), b.self() ); }
        friend sref operator|( sref a, sref b ) { return dom::op_or( a.self(), b.self() ); }
        friend sref operator^( sref a, sref b ) { return dom::op_xor( a.self(), b.self() ); }

        friend sref operator==( sref a, sref b ) { return dom::op_eq( a.self(), b.self() ); }
        friend sref operator!=( sref a, sref b ) { return dom::op_ne( a.self(), b.self() ); }
        friend sref operator>=( sref a, sref b ) { return ge( a.self(), b.self() ); }
        friend sref operator<=( sref a, sref b ) { return le( a.self(), b.self() ); }
        friend sref operator< ( sref a, sref b ) { return lt( a.self(), b.self() ); }
        friend sref operator> ( sref a, sref b ) { return gt( a.self(), b.self() ); }

        friend sref operator!( sref a ) { return dom::op_not( a.self() ); }
        friend sref operator&&( sref a, sref b ) { return dom::op_and( !!a.self(), !!b.self() ); }
        friend sref operator||( sref a, sref b ) { return dom::op_or ( !!a.self(), !!b.self() ); }

        friend sref operator<<( sref a, sref b ) { return dom::op_shl( a.self(), b.self() ); }
        friend sref operator>>( sref a, sref b ) { return shr( a.self(), b.self()); }

        friend sref zfit( sref a, int w ) { return dom::op_zfit( a.self(), w ); }

        static void assume( sref a )
        {
            if ( static_cast< bool >( a ) ) {} else { __lart_cancel(); }
        }
    };

} // namespace __lava