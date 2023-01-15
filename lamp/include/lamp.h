/*
 * (c) 2020 Henrich Lauko <xlauko@mail.muni.cz>
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

#ifdef __cplusplus
#include <string>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { void *ptr; } __lamp_ptr;

#ifndef __cplusplus
#define bool        _Bool
#define true        1
#define false       0
#endif

#define __lamp_null 0

#define __lamp_bool bool

typedef unsigned char      __lamp_i8;
typedef unsigned short     __lamp_i16;
typedef unsigned int       __lamp_i32;
typedef unsigned long long __lamp_i64;

typedef __lamp_i8 __lamp_bw;

typedef   float __lamp_f32;
typedef  double __lamp_f64;

typedef char      __lamp_si8;
typedef short     __lamp_si16;
typedef int       __lamp_si32;
typedef long long __lamp_si64;

__lamp_bool __lamp_lift_i1 ( __lamp_bool   v );
__lamp_i8  __lamp_lift_i8 ( __lamp_i8  v );
__lamp_i16 __lamp_lift_i16( __lamp_i16 v );
__lamp_i32 __lamp_lift_i32( __lamp_i32 v );
__lamp_i64 __lamp_lift_i64( __lamp_i64 v );

__lamp_si8  __lamp_lift_si8 ( __lamp_si8  v );
__lamp_si16 __lamp_lift_si16( __lamp_si16 v );
__lamp_si32 __lamp_lift_si32( __lamp_si32 v );
__lamp_si64 __lamp_lift_si64( __lamp_si64 v );

__lamp_f32 __lamp_lift_f32( __lamp_f32 v );
__lamp_f64 __lamp_lift_f64( __lamp_f64 v );

void *__lamp_lift_arr( void *v, __lamp_i32 s );
// char *__lamp_lift_str( char *s );

__lamp_ptr __lamp_wrap_i1 ( __lamp_bool     v );
__lamp_ptr __lamp_wrap_i8 ( __lamp_i8  v );
__lamp_ptr __lamp_wrap_i16( __lamp_i16 v );
__lamp_ptr __lamp_wrap_i32( __lamp_i32 v );
__lamp_ptr __lamp_wrap_i64( __lamp_i64 v );

__lamp_ptr __lamp_wrap_si8 ( __lamp_si8  v );
__lamp_ptr __lamp_wrap_si16( __lamp_si16 v );
__lamp_ptr __lamp_wrap_si32( __lamp_si32 v );
__lamp_ptr __lamp_wrap_si64( __lamp_si64 v );

__lamp_ptr __lamp_wrap_f32( __lamp_f32 v );
__lamp_ptr __lamp_wrap_f64( __lamp_f64 v );

__lamp_ptr __lamp_wrap_ptr( void *v );
void *__lamp_lift_ptr( void *v );

__lamp_bool __lamp_any_i1 ( void );
__lamp_i8   __lamp_any_i8 ( void );
__lamp_i16  __lamp_any_i16( void );
__lamp_i32  __lamp_any_i32( void );
__lamp_i64  __lamp_any_i64( void );

__lamp_i8    __lamp_any_range_i8(__lamp_i8 from, __lamp_i8 to);
__lamp_i16   __lamp_any_range_i16(__lamp_i16 from, __lamp_i16 to);
__lamp_i32   __lamp_any_range_i32(__lamp_i32 from, __lamp_i32 to);
__lamp_i64   __lamp_any_range_i64(__lamp_i64 from, __lamp_i64 to);

float    __lamp_any_f32  ( void );
double   __lamp_any_f64  ( void );
void    *__lamp_any_ptr  ( void );
char    *__lamp_any_array( void );

// __lamp_bool __lamp_lower_i1 ( __lamp_ptr v );
// __lamp_i8   __lamp_lower_i8 ( __lamp_ptr v );
// __lamp_i16  __lamp_lower_i16( __lamp_ptr v );
// __lamp_i32  __lamp_lower_i32( __lamp_ptr v );
// __lamp_i64  __lamp_lower_i64( __lamp_ptr v );

__lamp_ptr __lamp_alloca( __lamp_ptr size, __lamp_bw bw );

void __lamp_freeze( void *val, void *addr, __lamp_bw bytes );
__lamp_ptr __lamp_melt( void *addr, __lamp_bw bytes );

__lamp_ptr __lamp_join( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_meet( __lamp_ptr a, __lamp_ptr b );

__lamp_ptr __lamp_add ( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_sub ( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_mul ( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_sdiv( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_udiv( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_srem( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_urem( __lamp_ptr a, __lamp_ptr b );

__lamp_ptr __lamp_fadd( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_fsub( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_fmul( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_fdiv( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_frem( __lamp_ptr a, __lamp_ptr b );

__lamp_ptr __lamp_shl ( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_ashr( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_lshr( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_and ( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_or  ( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_xor ( __lamp_ptr a, __lamp_ptr b );

__lamp_ptr __lamp_eq ( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_ne ( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_ugt( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_uge( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_ult( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_ule( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_sgt( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_sge( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_slt( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_sle( __lamp_ptr a, __lamp_ptr b );

__lamp_ptr __lamp_foeq( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_fogt( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_foge( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_folt( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_fole( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_fone( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_ford( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_funo( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_fueq( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_fugt( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_fuge( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_fult( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_fule( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_fune( __lamp_ptr a, __lamp_ptr b );

__lamp_ptr __lamp_ffalse( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_ftrue ( __lamp_ptr a, __lamp_ptr b );

__lamp_ptr __lamp_concat ( __lamp_ptr a, __lamp_ptr b );
__lamp_ptr __lamp_trunc  ( __lamp_ptr a, __lamp_bw  b );
__lamp_ptr __lamp_fptrunc( __lamp_ptr a, __lamp_bw  b );
__lamp_ptr __lamp_sitofp ( __lamp_ptr a, __lamp_bw  b );
__lamp_ptr __lamp_uitofp ( __lamp_ptr a, __lamp_bw  b );
__lamp_ptr __lamp_zext   ( __lamp_ptr a, __lamp_bw  b );
__lamp_ptr __lamp_sext   ( __lamp_ptr a, __lamp_bw  b );
__lamp_ptr __lamp_fpext  ( __lamp_ptr a, __lamp_bw  b );
__lamp_ptr __lamp_fptosi ( __lamp_ptr a, __lamp_bw  b );
__lamp_ptr __lamp_fptoui ( __lamp_ptr a, __lamp_bw  b );

void __lamp_store( __lamp_ptr a, __lamp_ptr b, __lamp_bw w );
__lamp_ptr __lamp_load( __lamp_ptr a, __lamp_bw w );

__lamp_i8 __lamp_to_tristate( __lamp_ptr v );
__lamp_bool __lamp_to_bool( __lamp_ptr v );

void __lamp_assume( __lamp_ptr a, __lamp_bool c );
__lamp_ptr __lamp_extract( __lamp_ptr a, __lamp_bw s, __lamp_bw e );

// void __lamp_dealloca( void *p, __lamp_i64 size );
void __lamp_dump( void *twin );
void __lamp_value_dump( __lamp_ptr abstract );

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
    std::string __lamp_trace( void *twin );
#endif
