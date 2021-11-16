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
extern "C"
{
    __lamp_ptr __lamp_peek( void *addr );

    double __lamp_lifter_fabs( double a )
    {
        if  ( __lart_test_taint( *reinterpret_cast< uint8_t* >( &a ) ) ) {
            __lart_stash( __lamp_fn_fabs( { __lart_unstash() } ).ptr );
            return a;
        }

        return std::fabs(a);
    }

    float __lamp_lifter_fabsf( float a )
    {
        if  ( __lart_test_taint( *reinterpret_cast< uint8_t* >( &a ) ) ) {
            __lart_stash( __lamp_fn_fabs( { __lart_unstash() } ).ptr );
            return a;
        }

        return std::fabsf(a);
    }
}