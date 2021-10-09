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
 
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"

#define _SVC_NONDET(t,n,bw) t __VERIFIER_nondet_ ## n(void) { return __lamp_any_i ## bw(); }

extern "C" {

    uint8_t  __lamp_any_i1(void);
    uint8_t  __lamp_any_i8(void);
    uint16_t __lamp_any_i16(void);
    uint32_t __lamp_any_i32(void);
    uint64_t __lamp_any_i64(void);

    float    __lamp_any_f32(void);
    double   __lamp_any_f64(void);

    void* __lamp_any_ptr(void);

    _SVC_NONDET(bool, bool, 8)
    _SVC_NONDET(char, char, 8)
    _SVC_NONDET(unsigned char, uchar, 8)
    _SVC_NONDET(int, int, 32)
    _SVC_NONDET(short, short, 16)
    _SVC_NONDET(long, long, 64)
    _SVC_NONDET(unsigned short, ushort, 16)
    _SVC_NONDET(unsigned int, uint, 32)
    _SVC_NONDET(unsigned int, unsigned_int, 32)
    _SVC_NONDET(unsigned long, ulong, 64)
    _SVC_NONDET(unsigned, unsigned, 32)
    _SVC_NONDET(uint64_t, pointer, 64)

    const char* __VERIFIER_nondet_const_char_pointer(void)
    {
        return static_cast< char * >( __lamp_any_ptr() );
    }

    float  __VERIFIER_nondet_float(void)  { return __lamp_any_f32(); }
    double __VERIFIER_nondet_double(void) { return __lamp_any_f64(); }

}

#pragma clang diagnostic pop
