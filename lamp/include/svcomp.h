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

#include <lamp.h>

#ifdef __cplusplus
extern "C" {
#endif

    __lamp_bool __lamp_any_i1_traced (const char *file, unsigned int line, const char *func);
    __lamp_i8   __lamp_any_i8_traced (const char *file, unsigned int line, const char *func);
    __lamp_i16  __lamp_any_i16_traced(const char *file, unsigned int line, const char *func);
    __lamp_i32  __lamp_any_i32_traced(const char *file, unsigned int line, const char *func);
    __lamp_i64  __lamp_any_i64_traced(const char *file, unsigned int line, const char *func);
        
    __lamp_f32 __lamp_any_f32_traced(const char *file, unsigned int line, const char *func);
    __lamp_f64 __lamp_any_f64_traced(const char *file, unsigned int line, const char *func);
    void*      __lamp_any_ptr_traced(const char *file, unsigned int line, const char *func);

    #define __VERIFIER_nondet_bool() \
        __lamp_any_i1_traced(__FILE__, __LINE__, "__VERIFIER_nondet_bool")

    #define __VERIFIER_nondet_char() \
        __lamp_any_i8_traced(__FILE__, __LINE__, "__VERIFIER_nondet_char")
    
    #define __VERIFIER_nondet_uchar() \
        __lamp_any_i8_traced(__FILE__, __LINE__, "__VERIFIER_nondet_uchar")
    
    #define __VERIFIER_nondet_short() \
        __lamp_any_i16_traced(__FILE__, __LINE__, "__VERIFIER_nondet_short")
    
    #define __VERIFIER_nondet_ushort() \
        __lamp_any_i16_traced(__FILE__, __LINE__, "__VERIFIER_nondet_ushort")
    
    #define __VERIFIER_nondet_int() \
        __lamp_any_i32_traced(__FILE__, __LINE__, "__VERIFIER_nondet_int")

    #define __VERIFIER_nondet_uint() \
        __lamp_any_i32_traced(__FILE__, __LINE__, "__VERIFIER_nondet_uint")
    
    #define __VERIFIER_nondet_unsigned() \
        __lamp_any_i32_traced(__FILE__, __LINE__, "__VERIFIER_nondet_unsigned")
    
    #define __VERIFIER_nondet_unsigned_int() \
        __lamp_any_i32_traced(__FILE__, __LINE__, "__VERIFIER_nondet_unsigned_int")
    
    #define __VERIFIER_nondet_long() \
        __lamp_any_i64_traced(__FILE__, __LINE__, "__VERIFIER_nondet_long")
    
    #define __VERIFIER_nondet_ulong() \
        __lamp_any_i64_traced(__FILE__, __LINE__, "__VERIFIER_nondet_ulong")
    
    #define __VERIFIER_nondet_longlong() \
        __lamp_any_i64_traced(__FILE__, __LINE__, "__VERIFIER_nondet_longlong")
    
    #define __VERIFIER_nondet_ulonglong() \
        __lamp_any_i64_traced(__FILE__, __LINE__, "__VERIFIER_nondet_ulonglong")
    
    #define __VERIFIER_nondet_pointer() \
        __lamp_any_ptr_traced(__FILE__, __LINE__, "__VERIFIER_nondet_pointer")

    #define __VERIFIER_nondet_const_char_pointer() \
        __lamp_any_ptr_traced(__FILE__, __LINE__, "__VERIFIER_nondet_const_char_pointer")
    
    #define __VERIFIER_nondet_float() \
        __lamp_any_f32_traced(__FILE__, __LINE__, "__VERIFIER_nondet_float")
    
    #define __VERIFIER_nondet_double() \
        __lamp_any_f64_traced(__FILE__, __LINE__, "__VERIFIER_nondet_double")

#ifdef __cplusplus
}
#endif
