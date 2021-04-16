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

#include <stdint.h>
#include <stdbool.h>

#include <runtime/lart.h>

#ifdef __cplusplus
#include <string>
#endif

__lart_ignore_diagnostic

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { void *ptr; } __lamp_ptr;

typedef   float __lamp_f32;
typedef  double __lamp_f64;
typedef uint8_t __lamp_bw;

#define __lamp_export __flatten __noalias __ignore_args __ignore_return
#define __lamp_scalar __lamp_export __annotate( lart.abstract.return.scalar )
#define __lamp_pointer  __lamp_export __annotate( lart.abstract.return.pointer )

__lamp_scalar bool     __lamp_lift_i1 ( bool   v ) __lart_stub;
__lamp_scalar uint8_t  __lamp_lift_i8 ( uint8_t  v ) __lart_stub;
__lamp_scalar uint16_t __lamp_lift_i16( uint16_t v ) __lart_stub;
__lamp_scalar uint32_t __lamp_lift_i32( uint32_t v ) __lart_stub;
__lamp_scalar uint64_t __lamp_lift_i64( uint64_t v ) __lart_stub;

__lamp_scalar int8_t  __lamp_lift_si8 ( int8_t  v ) __lart_stub;
__lamp_scalar int16_t __lamp_lift_si16( int16_t v ) __lart_stub;
__lamp_scalar int32_t __lamp_lift_si32( int32_t v ) __lart_stub;
__lamp_scalar int64_t __lamp_lift_si64( int64_t v ) __lart_stub;

__lamp_scalar __lamp_f32 __lamp_lift_f32( __lamp_f32 v ) __lart_stub;
__lamp_scalar __lamp_f64 __lamp_lift_f64( __lamp_f64 v ) __lart_stub;

__lamp_pointer void *__lamp_lift_arr( void *v, uint32_t s ) __lart_stub;
__lamp_pointer char *__lamp_lift_str( char *s ) __lart_stub;

__lamp_ptr __lamp_wrap_i1 ( bool     v ) __lart_stub;
__lamp_ptr __lamp_wrap_i8 ( uint8_t  v ) __lart_stub;
__lamp_ptr __lamp_wrap_i16( uint16_t v ) __lart_stub;
__lamp_ptr __lamp_wrap_i32( uint32_t v ) __lart_stub;
__lamp_ptr __lamp_wrap_i64( uint64_t v ) __lart_stub;

__lamp_ptr __lamp_wrap_si8 ( int8_t  v ) __lart_stub;
__lamp_ptr __lamp_wrap_si16( int16_t v ) __lart_stub;
__lamp_ptr __lamp_wrap_si32( int32_t v ) __lart_stub;
__lamp_ptr __lamp_wrap_si64( int64_t v ) __lart_stub;

__lamp_ptr __lamp_wrap_f32( __lamp_f32 v ) __lart_stub;
__lamp_ptr __lamp_wrap_f64( __lamp_f64 v ) __lart_stub;

__lamp_ptr __lamp_wrap_ptr( void *v ) __lart_stub;
void *__lamp_lift_ptr( void *v ) __lart_stub;

uint8_t  __lamp_any_i8   ( void ) __lart_stub;
uint16_t __lamp_any_i16  ( void ) __lart_stub;
uint32_t __lamp_any_i32  ( void ) __lart_stub;
uint64_t __lamp_any_i64  ( void ) __lart_stub;
float    __lamp_any_f32  ( void ) __lart_stub;
double   __lamp_any_f64  ( void ) __lart_stub;
void    *__lamp_any_ptr  ( void ) __lart_stub;
char    *__lamp_any_array( void ) __lart_stub;

bool    __lamp_lower_i1 ( __lamp_ptr v ) __lart_stub;
uint8_t  __lamp_lower_i8 ( __lamp_ptr v ) __lart_stub;
uint16_t __lamp_lower_i16( __lamp_ptr v ) __lart_stub;
uint32_t __lamp_lower_i32( __lamp_ptr v ) __lart_stub;
uint64_t __lamp_lower_i64( __lamp_ptr v ) __lart_stub;

__lamp_ptr __lamp_alloca( __lamp_ptr size, __lamp_bw bw ) __lart_stub;

void __lamp_freeze( void *val, void *addr, __lamp_bw bw ) __lart_stub;

__lamp_ptr __lamp_join( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_meet( __lamp_ptr a, __lamp_ptr b ) __lart_stub;

__lamp_ptr __lamp_add ( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_sub ( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_mul ( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_sdiv( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_udiv( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_srem( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_urem( __lamp_ptr a, __lamp_ptr b ) __lart_stub;

__lamp_ptr __lamp_fadd( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_fsub( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_fmul( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_fdiv( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_frem( __lamp_ptr a, __lamp_ptr b ) __lart_stub;

__lamp_ptr __lamp_shl ( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_ashr( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_lshr( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_and ( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_or  ( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_xor ( __lamp_ptr a, __lamp_ptr b ) __lart_stub;

__lamp_ptr __lamp_eq ( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_ne ( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_ugt( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_uge( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_ult( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_ule( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_sgt( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_sge( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_slt( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_sle( __lamp_ptr a, __lamp_ptr b ) __lart_stub;

__lamp_ptr __lamp_foeq( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_fogt( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_foge( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_folt( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_fole( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_fone( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_ford( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_funo( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_fueq( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_fugt( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_fuge( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_fult( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_fule( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_fune( __lamp_ptr a, __lamp_ptr b ) __lart_stub;

__lamp_ptr __lamp_ffalse( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_ftrue ( __lamp_ptr a, __lamp_ptr b ) __lart_stub;

__lamp_ptr __lamp_concat ( __lamp_ptr a, __lamp_ptr b ) __lart_stub;
__lamp_ptr __lamp_trunc  ( __lamp_ptr a, __lamp_bw  b ) __lart_stub;
__lamp_ptr __lamp_fptrunc( __lamp_ptr a, __lamp_bw  b ) __lart_stub;
__lamp_ptr __lamp_sitofp ( __lamp_ptr a, __lamp_bw  b ) __lart_stub;
__lamp_ptr __lamp_uitofp ( __lamp_ptr a, __lamp_bw  b ) __lart_stub;
__lamp_ptr __lamp_zext   ( __lamp_ptr a, __lamp_bw  b ) __lart_stub;
__lamp_ptr __lamp_sext   ( __lamp_ptr a, __lamp_bw  b ) __lart_stub;
__lamp_ptr __lamp_fpext  ( __lamp_ptr a, __lamp_bw  b ) __lart_stub;
__lamp_ptr __lamp_fptosi ( __lamp_ptr a, __lamp_bw  b ) __lart_stub;
__lamp_ptr __lamp_fptoui ( __lamp_ptr a, __lamp_bw  b ) __lart_stub;

void __lamp_store( __lamp_ptr a, __lamp_ptr b, __lamp_bw w ) __lart_stub
__lamp_ptr __lamp_load( __lamp_ptr a, __lamp_bw w ) __lart_stub;

__lamp_export uint8_t __lamp_to_tristate( __lamp_ptr v ) __lart_stub;
__lamp_export bool __lamp_to_bool( __lamp_ptr v ) __lart_stub;

void __lamp_assume( __lamp_ptr a, bool c ) __lart_stub;
__lamp_ptr __lamp_extract( __lamp_ptr a, __lamp_bw s, __lamp_bw e ) __lart_stub;

__lamp_export void __lamp_dealloca( void *p, uint64_t size ) __lart_stub;

void __lamp_dump( void *twin );

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
std::string __lamp_trace( void *twin );
#endif

__lart_pop_diagnostic
