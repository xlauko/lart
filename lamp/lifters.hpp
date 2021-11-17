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

#include <cmath>
#include <stdio.h>

namespace lart
{
    bool tainted( void *ptr )
    {
        return __lart_test_taint( *reinterpret_cast< uint8_t* >( ptr ) );
    }

    void stash( __lamp_ptr ptr )
    {
        __lart_stash( ptr.ptr );
    }

    __lamp_ptr wrap( float v ) { return __lamp_wrap_float(v); }
    
    __lamp_ptr lift( double v ) { return __lamp_wrap_double(v); }

    template< typename T >
    __lamp_ptr arg_wrap( T v )
    {
        __lamp_ptr res{ __lart_unstash() };
        return !lart::tainted(&v) ? wrap(v) : res;
    }

    __lamp_ptr arg() { return { __lart_unstash() }; }
}


extern "C"
{
    // fabs
    double __lamp_lifter_fabs( double a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_fabs( lart::arg() ) );
            return a;
        }

        return std::fabs(a);
    }

    float __lamp_lifter_fabsf( float a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_fabs( lart::arg() ) );
            return a;
        }

        return std::fabsf(a);
    }

    // round
    double __lamp_lifter_round( double a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_round( lart::arg() ) );
            return a;
        }

        return std::round(a);
    }

    float __lamp_lifter_roundf( float a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_round( lart::arg() ) );
            return a;
        }

        return std::roundf(a);
    }

    // copysign
    double __lamp_lifter_copysign( double a, double b )
    {
        if  ( lart::tainted(&a) || lart::tainted(&b) ) {
            lart::stash( __lamp_fn_copysign( lart::arg_wrap(a), lart::arg_wrap(b) ) );
            return a;
        }

        return std::copysign(a, b);
    }

    float __lamp_lifter_copysignf( float a, float b )
    {
        if  ( lart::tainted(&a) || lart::tainted(&b) ) {
            lart::stash( __lamp_fn_copysign( lart::arg_wrap(a), lart::arg_wrap(b) ) );
            return a;
        }

        return std::copysignf(a, b);
    }

    // rint
    double __lamp_lifter_rint( double a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_rint( lart::arg() ) );
            return a;
        }

        return std::rint(a);
    }

    float __lamp_lifter_rintf( float a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_rint( lart::arg() ) );
            return a;
        }

        return std::rintf(a);
    }

    // sqrt - check __ieee754_sqrt
    // log - check __ieee754_log
    // log10 - check __ieee754_log10
    
    // asin - check __ieee754_asin

    // rint    
    // nearbyint

    // sin
    // cos
    // floor
    // ceil
    // trunc
    // copysign
}