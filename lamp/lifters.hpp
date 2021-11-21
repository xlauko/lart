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
#include <cstdlib>
#include <stdio.h>
#include <limits>

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

    __lamp_ptr wrap( unsigned long v ) { return (sizeof(unsigned long) == 32) ? __lamp_wrap_i32(v) :  __lamp_wrap_i64(v); }
    
    __lamp_ptr wrap( float v ) { return __lamp_wrap_float(v); }
    __lamp_ptr wrap( double v ) { return __lamp_wrap_double(v); }

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
    extern void __lart_stub_fault(const char *msg);

    bool __lamp_lifter_umull_overflow(unsigned long a, unsigned long b, unsigned long *c) {
        if  ( lart::tainted(&a) || lart::tainted(&b) ) {
            auto lhs = lart::arg_wrap(a);
            auto rhs = lart::arg_wrap(b);

            auto cmax = std::numeric_limits< unsigned long >::max();
            auto amax = (sizeof(unsigned long) == 32) ? __lamp_wrap_i32(cmax) :  __lamp_wrap_i64(cmax);
            auto zero = (sizeof(unsigned long) == 32) ? __lamp_wrap_i32(0) :  __lamp_wrap_i64(0);
            
            // if (lhs != 0 && max / lhs < rhs)
            auto iszero = __lamp_ne( lhs, zero );
            auto div    = __lamp_udiv( amax, lhs );
            auto fits   = __lamp_ult( div, rhs );
            auto cmp    = __lamp_and( iszero, fits );
            bool overflows = __lamp_to_bool(cmp);

            if ( overflows ) {
                __lamp_assume( cmp, true );
            } else {
                __lamp_assume( cmp, false );
                auto res = __lamp_mul( lhs, rhs );            
                __lamp_freeze( res, c, sizeof(unsigned long) );
            } 

            lart::stash( { nullptr } );
            return overflows;
        }

        return __builtin_umull_overflow(a, b, c);
    }

    bool __lamp_lifter_uaddl_overflow(unsigned long a, unsigned long b, unsigned long *c) {
        if  ( lart::tainted(&a) || lart::tainted(&b) ) {
            auto lhs = lart::arg_wrap(a);
            auto rhs = lart::arg_wrap(b);

            auto cmax = std::numeric_limits< unsigned long >::max();
            auto amax = (sizeof(unsigned long) == 32) ? __lamp_wrap_i32(cmax) :  __lamp_wrap_i64(cmax);

            // if (max - lhs < rhs)
            auto sub = __lamp_sub( amax, lhs );
            auto cmp = __lamp_ult( sub, rhs );
            auto overflows = __lamp_to_bool(cmp);
            
            if ( overflows ) {
                __lamp_assume( cmp, true );
            } else {
                __lamp_assume( cmp, false );
                auto res = __lamp_add( lhs, rhs );            
                __lamp_freeze( res, c, sizeof(unsigned long) );
            }

            lart::stash( { nullptr } );
            return overflows;
        }

        return __builtin_uaddl_overflow(a, b, c);
    }

    // malloc

    void* __lamp_lifter_malloc( size_t size )
    {
        if  ( lart::tainted(&size) ) {
            lart::stash( __lamp_fn_malloc( lart::arg() ) );
            void *value = 0;
            __lart_set_taint( &value, sizeof( value ) );
            return value;
        }
        return std::malloc( size );
    }
    
    // abs
    int __lamp_lifter_abs( int a, bool /* poison */ )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_abs( lart::arg() ) );
            return a;
        }

        return std::abs(a);
    }

    long __lamp_lifter_labs( long a, bool /* poison */ )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_abs( lart::arg() ) );
            return a;
        }

        return std::labs(a);
    }

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

    // fmod
    double __lamp_lifter_fmod( double a, double b )
    {
        if  ( lart::tainted(&a) || lart::tainted(&b) ) {
            lart::stash( __lamp_fn_fmod( lart::arg_wrap(a), lart::arg_wrap(b) ) );
            return a;
        }

        return std::fmod(a, b);
    }

    float __lamp_lifter_fmodf( float a, float b )
    {
        if  ( lart::tainted(&a) || lart::tainted(&b) ) {
            lart::stash( __lamp_fn_fmod( lart::arg_wrap(a), lart::arg_wrap(b) ) );
            return a;
        }

        return std::fmodf(a, b);
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

    // ceil
    double __lamp_lifter_ceil( double a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_ceil( lart::arg() ) );
            return a;
        }

        return std::ceil(a);
    }

    float __lamp_lifter_ceilf( float a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_ceil( lart::arg() ) );
            return a;
        }

        return std::ceilf(a);
    }
    
    // trunc
    double __lamp_lifter_trunc( double a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_trunc( lart::arg() ) );
            return a;
        }

        return std::trunc(a);
    }

    float __lamp_lifter_truncf( float a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_trunc( lart::arg() ) );
            return a;
        }

        return std::truncf(a);
    }

    // isnan
    int __lamp_lifter_isnan( double a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_isnan( lart::arg() ) );
            return a;
        }

        return std::isnan(a);
    }

    int __lamp_lifter_isnanf( float a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_isnan( lart::arg() ) );
            return a;
        }

        return std::isnan(a);
    }

    int __lamp_lifter_isnanl( long double a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_isnan( lart::arg() ) );
            return a;
        }

        return std::isnan(a);
    }


    // isinf
    int __lamp_lifter_isinf( double a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_isinf( lart::arg() ) );
            return a;
        }

        return std::isinf(a);
    }

    int __lamp_lifter_isinff( float a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_isinf( lart::arg() ) );
            return a;
        }

        return std::isinf(a);
    }

    int __lamp_lifter_isinfl( long double a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_isinf( lart::arg() ) );
            return a;
        }

        return std::isinf(a);
    }
    
    // isfinite
    int __lamp_lifter_finite( double a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_isfinite( lart::arg() ) );
            return a;
        }

        return std::isfinite(a);
    }

    int __lamp_lifter_finitef( float a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_isfinite( lart::arg() ) );
            return a;
        }

        return std::isfinite(a);
    }

    int __lamp_lifter_finitel( long double a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_isfinite( lart::arg() ) );
            return a;
        }

        return std::isfinite(a);
    }

    int __lamp_lifter_isfinite( double a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_isfinite( lart::arg() ) );
            return a;
        }

        return std::isfinite(a);
    }

    int __lamp_lifter_isfinitef( float a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_isfinite( lart::arg() ) );
            return a;
        }

        return std::isfinite(a);
    }

    int __lamp_lifter_isfinitel( long double a )
    {
        if  ( lart::tainted(&a) ) {
            lart::stash( __lamp_fn_isfinite( lart::arg() ) );
            return a;
        }

        return std::isfinite(a);
    }

}