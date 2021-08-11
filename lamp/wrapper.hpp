/*
 * (c) 2020 Petr Ročkai <code@fixp.eu>
 * (c) 2020 Henrich Lauko <xlauko@mail.muni.cz>
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

/* This file is supposed to be included, but is not really a header. Instead,
 * it implements the LART-facing metadomain wrapper. Each metadomain should be
 * compiled into a separate bitcode file which contains exactly one copy
 * (build) of this file. See e.g. trivial.cpp for an example. */

#include <lava/support/base.hpp> /* iN */
#include <lamp/support/storage.hpp> /* domain_ref */
#include <lava/constant.hpp>

#include <runtime/lart.h>

#ifdef __lart_cpp_runtime
    #include <string>
#endif

typedef struct { void *ptr; } __lamp_ptr;

using dom = __lamp::meta_domain;
using bw = __lava::bitwidth_t;

using namespace __lava; /* iN */

using ref = domain_ref< dom >;

struct wrapper
{
    template< typename op_t >
    __inline static __lamp_ptr wrap( op_t op ) { return { op().disown() }; }

    template< typename op_t, typename... args_t >
    __inline static __lamp_ptr wrap( op_t op, __lamp_ptr arg, args_t ...args )
    {
        ref a( arg.ptr );
        return wrap( [&]( auto ...args_ ) __inline { return op( a, args_... ); }, args... );
    }

    template< typename op_t, typename arg_t, typename... args_t >
    __inline static auto wrap( op_t op, const arg_t &arg, args_t ...args )
        -> std::enable_if_t< !std::is_same_v< arg_t, dom >, __lamp_ptr >
    {
        return wrap( [&] ( auto ...args_ ) __inline { return op( arg, args_... ); }, args... );
    }

    template< typename op_t >
    __inline static void wrap_void( op_t op ) { op(); }

    template< typename op_t, typename... args_t >
    __inline static void wrap_void( op_t op, __lamp_ptr arg, args_t ...args )
    {
        ref a( arg.ptr );
        wrap_void( [&]( auto ...args_ ) __inline { op( a, args_... ); }, args... );
    }

    template< typename op_t, typename arg_t, typename... args_t >
    __inline static auto wrap_void( op_t op, const arg_t &arg, args_t ...args )
        -> std::enable_if_t< !std::is_same_v< arg_t, dom >, void >
    {
        wrap_void( [&] ( auto ...args_ ) __inline { op( arg, args_... ); }, args... );
    }
};

template< typename... args_t >
__inline static __lamp_ptr wrap( const args_t & ...args ) { return wrapper::wrap( args... ); }

template< typename... args_t >
__inline static void wrap_void( const args_t & ...args ) { wrapper::wrap_void( args... ); }

template< typename type >
inline type defualt_tainted_value() {
    uint64_t value = 0;
    __lart_set_taint( &value, sizeof( value ) );
    if constexpr ( std::is_pointer_v< type > )
        return reinterpret_cast< type >( value );
    else
        return static_cast< type >( value );
}

template< typename op_t, typename arg_t, typename... args_t >
static arg_t lift( op_t op, arg_t arg, args_t... args )
{
    __lart_stash( op( arg, args... ).disown() );
    __lart_set_taint( &arg, sizeof( arg ) );
    return arg;
}

template< typename dom, typename type >
static auto any()
{
    __lart_stash( dom::template any< type >().disown() );
    return defualt_tainted_value< type >();
}

// template< typename T >
// static T lower( __lamp_ptr v )
// {
//     return cast< false /* unsigned */, bitwidth_v< T > >( dom::lower( ref( v.ptr ) )->value );
// }

extern "C"
{
    i1  __lamp_lift_i1 ( i1  v )     { return lift( dom::lift_i1,  v ); }
    i8  __lamp_lift_i8 ( i8  v )     { return lift( dom::lift_i8,  v ); }
    i16 __lamp_lift_i16( i16 v )     { return lift( dom::lift_i16, v ); }
    i32 __lamp_lift_i32( i32 v )     { return lift( dom::lift_i32, v ); }
    i64 __lamp_lift_i64( i64 v )     { return lift( dom::lift_i64, v ); }

    si8  __lamp_lift_si8 ( si8  v )  { return lift( dom::lift_si8,  v ); }
    si16 __lamp_lift_si16( si16 v )  { return lift( dom::lift_si16, v ); }
    si32 __lamp_lift_si32( si32 v )  { return lift( dom::lift_si32, v ); }
    si64 __lamp_lift_si64( si64 v )  { return lift( dom::lift_si64, v ); }

    f32 __lamp_lift_f32( f32 v )     { return lift( dom::lift_f32, v ); }
    f64 __lamp_lift_f64( f64 v )     { return lift( dom::lift_f64, v ); }

    __lamp_ptr __lamp_wrap_i1 ( i1  v )     { return wrap( dom::lift_i1,  v ); }
    __lamp_ptr __lamp_wrap_i8 ( i8  v )     { return wrap( dom::lift_i8,  v ); }
    __lamp_ptr __lamp_wrap_i16( i16 v )     { return wrap( dom::lift_i16, v ); }
    __lamp_ptr __lamp_wrap_i32( i32 v )     { return wrap( dom::lift_i32, v ); }
    __lamp_ptr __lamp_wrap_i64( i64 v )     { return wrap( dom::lift_i64, v ); }

    __lamp_ptr __lamp_wrap_si8 ( si8  v )     { return wrap( dom::lift_si8,  v ); }
    __lamp_ptr __lamp_wrap_si16( si16 v )     { return wrap( dom::lift_si16, v ); }
    __lamp_ptr __lamp_wrap_si32( si32 v )     { return wrap( dom::lift_si32, v ); }
    __lamp_ptr __lamp_wrap_si64( si64 v )     { return wrap( dom::lift_si64, v ); }

    __lamp_ptr __lamp_wrap_f32( f32 v )     { return wrap( dom::lift_f32, v ); }
    __lamp_ptr __lamp_wrap_f64( f64 v )     { return wrap( dom::lift_f64, v ); }

    __lamp_ptr  __lamp_wrap_ptr( void *v )   { return wrap( dom::lift_ptr, v ); }

    void* __lamp_lift_ptr( void *v ) { return lift( dom::lift_ptr, v ); }
    void* __lamp_lift_arr( void *v, i32 s ) { return lift( dom::lift_arr, v, s ); }

    // // i1  __lamp_lower_i1 ( __lamp_ptr v ) { return lower<  i1 >( v ); }
    // // i8  __lamp_lower_i8 ( __lamp_ptr v ) { return lower<  i8 >( v ); }
    // // i16 __lamp_lower_i16( __lamp_ptr v ) { return lower< i16 >( v ); }
    // // i32 __lamp_lower_i32( __lamp_ptr v ) { return lower< i32 >( v ); }
    // // i64 __lamp_lower_i64( __lamp_ptr v ) { return lower< i64 >( v ); }

    i8    __lamp_any_i8()    { return any< dom, i8  >(); }
    i16   __lamp_any_i16()   { return any< dom, i16 >(); }
    i32   __lamp_any_i32()   { return any< dom, i32 >(); }
    i64   __lamp_any_i64()   { return any< dom, i64 >(); }

    f32   __lamp_any_f32()   { return any< dom, f32 >(); }
    f64   __lamp_any_f64()   { return any< dom, f64 >(); }

    void* __lamp_any_ptr() { return any< dom, void * >(); }
    char* __lamp_any_array() { return any< dom, char * >(); }

    __lamp_ptr __lamp_alloca( __lamp_ptr size, bw w )
    {
        return wrap( []( const auto &... x ) { return dom::op_alloca( x... ); }, size, w );
    }

    /*void __lamp_freeze( void *val, void *addr, bw size )
    {
        __lamp_freeze_impl( val, addr, size );
    }

    __lamp_ptr __lamp_melt( void *addr, bw size )
    {
        return { __lamp_melt_impl( addr, size ).disown() };
    }*/

    __lamp_ptr __lamp_join( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_join, a, b ); }
    __lamp_ptr __lamp_meet( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_meet, a, b ); }

    __lamp_ptr __lamp_add ( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_add, a, b ); }
    __lamp_ptr __lamp_sub ( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_sub, a, b ); }
    __lamp_ptr __lamp_mul ( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_mul, a, b ); }
    __lamp_ptr __lamp_sdiv( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_sdiv, a, b ); }
    __lamp_ptr __lamp_udiv( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_udiv, a, b ); }
    __lamp_ptr __lamp_srem( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_srem, a, b ); }
    __lamp_ptr __lamp_urem( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_urem, a, b ); }

    __lamp_ptr __lamp_fadd( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_fadd, a, b ); }
    __lamp_ptr __lamp_fsub( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_fsub, a, b ); }
    __lamp_ptr __lamp_fmul( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_fmul, a, b ); }
    __lamp_ptr __lamp_fdiv( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_fdiv, a, b ); }
    __lamp_ptr __lamp_frem( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_frem, a, b ); }

    __lamp_ptr __lamp_shl ( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_shl,  a, b ); }
    __lamp_ptr __lamp_ashr( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_ashr, a, b ); }
    __lamp_ptr __lamp_lshr( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_lshr, a, b ); }
    __lamp_ptr __lamp_and ( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_and,  a, b ); }
    __lamp_ptr __lamp_or  ( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_or,   a, b ); }
    __lamp_ptr __lamp_xor ( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_xor,  a, b ); }

    __lamp_ptr __lamp_eq ( __lamp_ptr a, __lamp_ptr b )  { return wrap( dom::op_eq,  a, b ); }
    __lamp_ptr __lamp_ne ( __lamp_ptr a, __lamp_ptr b )  { return wrap( dom::op_ne,  a, b ); }
    __lamp_ptr __lamp_ugt( __lamp_ptr a, __lamp_ptr b )  { return wrap( dom::op_ugt, a, b ); }
    __lamp_ptr __lamp_uge( __lamp_ptr a, __lamp_ptr b )  { return wrap( dom::op_uge, a, b ); }
    __lamp_ptr __lamp_ult( __lamp_ptr a, __lamp_ptr b )  { return wrap( dom::op_ult, a, b ); }
    __lamp_ptr __lamp_ule( __lamp_ptr a, __lamp_ptr b )  { return wrap( dom::op_ule, a, b ); }
    __lamp_ptr __lamp_sgt( __lamp_ptr a, __lamp_ptr b )  { return wrap( dom::op_sgt, a, b ); }
    __lamp_ptr __lamp_sge( __lamp_ptr a, __lamp_ptr b )  { return wrap( dom::op_sge, a, b ); }
    __lamp_ptr __lamp_slt( __lamp_ptr a, __lamp_ptr b )  { return wrap( dom::op_slt, a, b ); }
    __lamp_ptr __lamp_sle( __lamp_ptr a, __lamp_ptr b )  { return wrap( dom::op_sle, a, b ); }

    __lamp_ptr __lamp_foeq( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_foeq, a, b ); }
    __lamp_ptr __lamp_fogt( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_fogt, a, b ); }
    __lamp_ptr __lamp_foge( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_foge, a, b ); }
    __lamp_ptr __lamp_folt( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_folt, a, b ); }
    __lamp_ptr __lamp_fole( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_fole, a, b ); }
    __lamp_ptr __lamp_fone( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_fone, a, b ); }
    __lamp_ptr __lamp_ford( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_ford, a, b ); }
    __lamp_ptr __lamp_funo( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_funo, a, b ); }
    __lamp_ptr __lamp_fueq( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_fueq, a, b ); }
    __lamp_ptr __lamp_fugt( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_fugt, a, b ); }
    __lamp_ptr __lamp_fuge( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_fuge, a, b ); }
    __lamp_ptr __lamp_fult( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_fult, a, b ); }
    __lamp_ptr __lamp_fule( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_fule, a, b ); }
    __lamp_ptr __lamp_fune( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_fune, a, b ); }

    __lamp_ptr __lamp_ffalse( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_ffalse, a, b ); }
    __lamp_ptr __lamp_ftrue ( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_ftrue,  a, b ); }

    __lamp_ptr __lamp_concat ( __lamp_ptr a, __lamp_ptr b ) { return wrap( dom::op_concat,  a, b ); }
    __lamp_ptr __lamp_trunc  ( __lamp_ptr a, bw  b ) { return wrap( dom::op_trunc,   a, b ); }
    __lamp_ptr __lamp_fptrunc( __lamp_ptr a, bw  b ) { return wrap( dom::op_fptrunc, a, b ); }
    __lamp_ptr __lamp_sitofp ( __lamp_ptr a, bw  b ) { return wrap( dom::op_sitofp,  a, b ); }
    __lamp_ptr __lamp_uitofp ( __lamp_ptr a, bw  b ) { return wrap( dom::op_uitofp,  a, b ); }
    __lamp_ptr __lamp_zext   ( __lamp_ptr a, bw  b ) { return wrap( dom::op_zext,    a, b ); }
    __lamp_ptr __lamp_zfit   ( __lamp_ptr a, bw  b ) { return wrap( dom::op_zfit,    a, b ); }
    __lamp_ptr __lamp_sext   ( __lamp_ptr a, bw  b ) { return wrap( dom::op_sext,    a, b ); }
    __lamp_ptr __lamp_fpext  ( __lamp_ptr a, bw  b ) { return wrap( dom::op_fpext,   a, b ); }
    __lamp_ptr __lamp_fptosi ( __lamp_ptr a, bw  b ) { return wrap( dom::op_fptosi,  a, b ); }
    __lamp_ptr __lamp_fptoui ( __lamp_ptr a, bw  b ) { return wrap( dom::op_fptoui,  a, b ); }

    void __lamp_store( __lamp_ptr a, __lamp_ptr b, bw w )
    {
        wrap_void( []( const auto &... x ) { dom::op_store( x... ); }, a, b, w );
    }

    __lamp_ptr __lamp_load( __lamp_ptr a, bw w ) { return wrap( dom::op_load, a, w ); }

    uint8_t __lamp_to_tristate( __lamp_ptr v )
    {
        return dom::to_tristate( ref( v.ptr ) ).value;
    }

    bool __lamp_to_bool( __lamp_ptr v )
    {
        bool lowered = __lava::lower( __lamp_to_tristate( v ) );
        __lart_set_taint( &lowered, sizeof( lowered ) );
        return lowered;
    }

    void __lamp_assume( __lamp_ptr a, bool c ) { dom::assume( ref( a.ptr ), c ); }
    __lamp_ptr __lamp_extract( __lamp_ptr a, bw s, bw e ) { return wrap( dom::op_extract, a, s, e ); }

    //void __lamp_dealloca( void * addr, uint64_t size ) { __lamp_dealloca_impl( addr, size ); }


    __lamp_ptr __lamp_copy( __lamp_ptr p )
    {
        return { ref( p.ptr ).clone().disown() };
    }

    void __lamp_dump( void *twin )
    {
        if ( twin && __lart_test_taint( *static_cast< uint8_t* >( twin ) ) ) {
            ref a( __lart_melt( twin, 0 ) );
            return dom::dump( a ); // TODO size?
        }
        printf( "concrete\n" );
    }
}

#ifdef __lart_cpp_runtime
    std::string __lamp_trace( void *twin )
    {
        if ( twin && __lart_test_taint( *static_cast< uint8_t* >( twin ) ) ) {
            ref a( __lart_melt( twin, 0 ) );
            return dom::trace( a ); // TODO size?
        }
        return "concrete";
    }
#endif
