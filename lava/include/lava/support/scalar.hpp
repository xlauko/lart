/*
 * (c) 2019 Henrich Lauko <xlauko@mail.muni.cz>
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

#pragma once

#include <lava/support/tristate.hpp>
#include <lava/support/base.hpp>

#include <runtime/lart.h>

namespace __lava
{
    // c++ wrapper for scalar domains

    template< typename dom, bool _signed = false, bool floating = false >
    struct scalar
    {
        static_assert( !(floating && !_signed) && "unsigned floats are not permitted" );

        dom _value;

        scalar( dom &&val ) : _value( std::move( val ) ) {}
        scalar( const dom &val ) : _value( val ) {}
        operator dom&() { return _value; }
        operator const dom&() const { return _value; }

        explicit operator bool()
        {
            auto v = dom::to_tristate( _value );

            switch ( v.value )
            {
                case tristate::false_value:  return false;
                case tristate::true_value: return true;
                case tristate::maybe_value:
                    bool rv = __lart_choose( 2 );
                    dom::assume( _value, rv );
                    return rv;
            }
        }

        template< typename type > static scalar lift( type val ) { return { dom::lift( val ) }; }
        template< typename type > static scalar any() { return { dom::template any< type >() }; }

        template< typename type >
        static scalar any(type from, type to) { return { dom::any(from, to) }; }

        template< typename type > static scalar concat( type l, type r ) { return { dom::op_concat( l, r ) }; }

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

        using sref = const scalar &;

        scalar operator+( sref o ) const { return add( _value, o._value ); }
        scalar operator-( sref o ) const { return sub( _value, o._value ); }
        scalar operator*( sref o ) const { return mul( _value, o._value ); }
        scalar operator/( sref o ) const { return div( _value, o._value ); }
        scalar operator%( sref o ) const { return rem( _value, o._value ); }

        scalar operator&( sref o ) const { return dom::op_and( _value, o._value ); }
        scalar operator|( sref o ) const { return dom::op_or( _value, o._value ); }
        scalar operator^( sref o ) const { return dom::op_xor( _value, o._value ); }

        scalar operator==( sref o ) const { return dom::op_eq( _value, o._value ); }
        scalar operator!=( sref o ) const { return dom::op_ne( _value, o._value ); }
        scalar operator>=( sref o ) const { return ge( _value, o._value ); }
        scalar operator<=( sref o ) const { return le( _value, o._value ); }
        scalar operator< ( sref o ) const { return lt( _value, o._value ); }
        scalar operator> ( sref o ) const { return gt( _value, o._value ); }

        scalar operator!() const { return dom::op_not( _value ); }
        scalar operator&&( sref o ) const { return dom::op_and( !!*this, !!o ); }
        scalar operator||( sref o ) const { return dom::op_or ( !!*this, !!o ); }

        scalar operator<<( sref o ) const { return dom::op_shl( _value, o._value ); }
        scalar operator>>( sref o ) const { return shr( _value, o._value); }

        scalar zfit( int w ) const { return dom::op_zfit( _value, w ); }

        static void assume( scalar value )
        {
            if ( static_cast< bool >( value ) ) {} else { __lart_cancel(); }
        }
    };
} // namespace __lava
