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

#define __lamp_null 0

#define __lamp_bool _Bool

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

__lamp_f32 __lamp_lift_float( __lamp_f32 v );
__lamp_f64 __lamp_lift_double( __lamp_f64 v );

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

__lamp_ptr __lamp_wrap_float( __lamp_f32 v );
__lamp_ptr __lamp_wrap_double( __lamp_f64 v );

__lamp_ptr __lamp_wrap_ptr( void *v );
void *__lamp_lift_ptr( void *v );

__lamp_bool __lamp_any_i1 ( void );
__lamp_i8   __lamp_any_i8 ( void );
__lamp_i16  __lamp_any_i16( void );
__lamp_i32  __lamp_any_i32( void );
__lamp_i64  __lamp_any_i64( void );

float    __lamp_any_float  ( void );
double   __lamp_any_double  ( void );
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

__lamp_ptr __lamp_fneg ( __lamp_ptr a );

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

__lamp_bool __lamp_lifter_umull_overflow(unsigned long a, unsigned long b, unsigned long *r);
__lamp_bool __lamp_lifter_uaddl_overflow(unsigned long a, unsigned long b, unsigned long *r);

__lamp_ptr __lamp_fn_malloc( __lamp_ptr size );
void* __lamp_lifter_malloc( unsigned size );

__lamp_ptr __lamp_fn_abs( __lamp_ptr a );
int __lamp_lifter_abs( int, __lamp_bool );
long __lamp_lifter_labs( long, __lamp_bool );

__lamp_ptr __lamp_fn_fabs( __lamp_ptr a );
double __lamp_lifter_fabs( double );
float __lamp_lifter_fabsf( float );

__lamp_ptr __lamp_fn_round( __lamp_ptr a );
double __lamp_lifter_round( double );
float __lamp_lifter_roundf( float );

__lamp_ptr __lamp_fn_copysing( __lamp_ptr a, __lamp_ptr b );
double __lamp_lifter_copysign( double, double );
float __lamp_lifter_copysignf( float, float );

__lamp_ptr __lamp_fn_fmod( __lamp_ptr a, __lamp_ptr b );
double __lamp_lifter_fmod( double, double );
float __lamp_lifter_fmodf( float, float );

__lamp_ptr __lamp_fn_rint( __lamp_ptr a );
double __lamp_lifter_rint( double );
float __lamp_lifter_rintf( float );

__lamp_ptr __lamp_fn_ceil( __lamp_ptr a );
double __lamp_lifter_ceil( double );
float __lamp_lifter_ceilf( float );

__lamp_ptr __lamp_fn_trunc( __lamp_ptr a );
double __lamp_lifter_trunc( double );
float __lamp_lifter_truncf( float );

__lamp_ptr __lamp_fn_isnan( __lamp_ptr a );
int __lamp_lifter_isnan( double );
int __lamp_lifter_isnanf( float );
int __lamp_lifter_isnanl( long double );

__lamp_ptr __lamp_fn_isinf( __lamp_ptr a );
int __lamp_lifter_isinf( double );
int __lamp_lifter_isinff( float );
int __lamp_lifter_isinfl( long double );

__lamp_ptr __lamp_fn_isfinite( __lamp_ptr a );
int __lamp_lifter_finite( double );
int __lamp_lifter_finitef( float );
int __lamp_lifter_finitel( long double );
int __lamp_lifter_isfinite( double );
int __lamp_lifter_isfinitef( float );
int __lamp_lifter_isfinitel( long double );

void __lamp_store( __lamp_ptr a, __lamp_ptr b, __lamp_bw w );
__lamp_ptr __lamp_load( __lamp_ptr a, __lamp_bw w );

__lamp_i8 __lamp_to_tristate( __lamp_ptr v );
__lamp_bool __lamp_to_bool( __lamp_ptr v );

void __lamp_assume( __lamp_ptr a, __lamp_bool c );
__lamp_ptr __lamp_extract( __lamp_ptr a, __lamp_bw s, __lamp_bw e );

// void __lamp_dealloca( void *p, __lamp_i64 size );

void __lamp_dump( void *twin );

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// std::string __lamp_trace( void *twin );
#endif

void __lamp_stub()
{
    __lamp_ptr ptr;
    __lamp_lift_i1 (0);
    __lamp_lift_i8 (0);
    __lamp_lift_i16(0);
    __lamp_lift_i32(0);
    __lamp_lift_i64(0);

    __lamp_lift_si8 (0);
    __lamp_lift_si16(0);
    __lamp_lift_si32(0);
    __lamp_lift_si64(0);

    __lamp_lift_float(0);
    __lamp_lift_double(0);

    __lamp_lift_arr( __lamp_null, 0 );
    // __lamp_lift_str( __lamp_null );

    __lamp_wrap_i1 (0);
    __lamp_wrap_i8 (0);
    __lamp_wrap_i16(0);
    __lamp_wrap_i32(0);
    __lamp_wrap_i64(0);

    __lamp_wrap_si8 (0);
    __lamp_wrap_si16(0);
    __lamp_wrap_si32(0);
    __lamp_wrap_si64(0);
    __lamp_wrap_float(0);
    __lamp_wrap_double(0);

    __lamp_wrap_ptr( __lamp_null );
    __lamp_lift_ptr( __lamp_null );

    __lamp_any_i1   ();
    __lamp_any_i8   ();
    __lamp_any_i16  ();
    __lamp_any_i32  ();
    __lamp_any_i64  ();
    __lamp_any_float  ();
    __lamp_any_double  ();
    __lamp_any_ptr  ();
    __lamp_any_array();

    // __lamp_lower_i1 ( ptr );
    // __lamp_lower_i8 ( ptr );
    // __lamp_lower_i16( ptr );
    // __lamp_lower_i32( ptr );
    // __lamp_lower_i64( ptr );

    __lamp_alloca( ptr, 0 );

    __lamp_freeze( __lamp_null, __lamp_null, 0 );
    __lamp_melt( __lamp_null, 0 );

    __lamp_join( ptr, ptr );
    __lamp_meet( ptr, ptr );

    __lamp_fneg( ptr );

    __lamp_add ( ptr, ptr );
    __lamp_sub ( ptr, ptr );
    __lamp_mul ( ptr, ptr );
    __lamp_sdiv( ptr, ptr );
    __lamp_udiv( ptr, ptr );
    __lamp_srem( ptr, ptr );
    __lamp_urem( ptr, ptr );

    __lamp_fadd( ptr, ptr );
    __lamp_fsub( ptr, ptr );
    __lamp_fmul( ptr, ptr );
    __lamp_fdiv( ptr, ptr );
    __lamp_frem( ptr, ptr );

    __lamp_shl ( ptr, ptr );
    __lamp_ashr( ptr, ptr );
    __lamp_lshr( ptr, ptr );
    __lamp_and ( ptr, ptr );
    __lamp_or  ( ptr, ptr );
    __lamp_xor ( ptr, ptr );

    __lamp_eq ( ptr, ptr );
    __lamp_ne ( ptr, ptr );
    __lamp_ugt( ptr, ptr );
    __lamp_uge( ptr, ptr );
    __lamp_ult( ptr, ptr );
    __lamp_ule( ptr, ptr );
    __lamp_sgt( ptr, ptr );
    __lamp_sge( ptr, ptr );
    __lamp_slt( ptr, ptr );
    __lamp_sle( ptr, ptr );

    __lamp_foeq( ptr, ptr );
    __lamp_fogt( ptr, ptr );
    __lamp_foge( ptr, ptr );
    __lamp_folt( ptr, ptr );
    __lamp_fole( ptr, ptr );
    __lamp_fone( ptr, ptr );
    __lamp_ford( ptr, ptr );
    __lamp_funo( ptr, ptr );
    __lamp_fueq( ptr, ptr );
    __lamp_fugt( ptr, ptr );
    __lamp_fuge( ptr, ptr );
    __lamp_fult( ptr, ptr );
    __lamp_fule( ptr, ptr );
    __lamp_fune( ptr, ptr );

    __lamp_ffalse( ptr, ptr );
    __lamp_ftrue ( ptr, ptr );

    __lamp_concat ( ptr, ptr );
    __lamp_trunc  ( ptr, 0 );
    __lamp_fptrunc( ptr, 0 );
    __lamp_sitofp ( ptr, 0 );
    __lamp_uitofp ( ptr, 0 );
    __lamp_zext   ( ptr, 0 );
    __lamp_sext   ( ptr, 0 );
    __lamp_fpext  ( ptr, 0 );
    __lamp_fptosi ( ptr, 0 );
    __lamp_fptoui ( ptr, 0 );

    __lamp_store( ptr, ptr, 0 );
    __lamp_load( ptr, 0 );

    __lamp_to_tristate( ptr );
    __lamp_to_bool( ptr );

    __lamp_assume( ptr, 0 );
    __lamp_extract( ptr, 0, 0 );

    __lamp_fn_malloc( ptr );
    __lamp_lifter_malloc( 0 );

    __lamp_lifter_umull_overflow( 0, 0, __lamp_null );
    __lamp_lifter_uaddl_overflow( 0, 0, __lamp_null );
    
    __lamp_fn_abs( ptr );
    __lamp_lifter_abs( 0, 0 );
    __lamp_lifter_labs( 0l, 0 );

    __lamp_fn_fabs( ptr );
    __lamp_lifter_fabs( 0.0 );
    __lamp_lifter_fabsf( 0.0f );

    __lamp_fn_round( ptr );
    __lamp_lifter_round( 0.0 );
    __lamp_lifter_roundf( 0.0f );

    __lamp_fn_rint( ptr );
    __lamp_lifter_rint( 0.0 );
    __lamp_lifter_rintf( 0.0f );

    __lamp_fn_copysing( ptr, ptr );
    __lamp_lifter_copysign( 0.0, 0.0 );
    __lamp_lifter_copysignf( 0.0f, 0.0f );

    __lamp_fn_fmod( ptr, ptr );
    __lamp_lifter_fmod( 0.0, 0.0 );
    __lamp_lifter_fmodf( 0.0f, 0.0f );

    __lamp_fn_ceil( ptr );
    __lamp_lifter_ceil( 0.0 );
    __lamp_lifter_ceilf( 0.0f );

    __lamp_fn_trunc( ptr );
    __lamp_lifter_trunc( 0.0 );
    __lamp_lifter_truncf( 0.0f );

    __lamp_fn_isnan( ptr );
    __lamp_lifter_isnan( 0.0 );
    __lamp_lifter_isnanf( 0.0f );
    __lamp_lifter_isnanl( 0.0 );

    __lamp_fn_isinf( ptr );
    __lamp_lifter_isinf( 0.0 );
    __lamp_lifter_isinff( 0.0f );
    __lamp_lifter_isinfl( 0.0 );

    __lamp_fn_isfinite( ptr );
    __lamp_lifter_finite( 0.0 );
    __lamp_lifter_finitef( 0.0f );
    __lamp_lifter_finitel( 0.0 );
    __lamp_lifter_isfinite( 0.0 );
    __lamp_lifter_isfinitef( 0.0f );
    __lamp_lifter_isfinitel( 0.0 );

    // __lamp_dealloca( __lamp_null, 0 );

    __lamp_dump( __lamp_null );

    #ifdef __cplusplus
    // __lamp_trace( __lamp_null );
    #endif
}
