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

#include <cstdint>
#include <cstddef>

// fix lava release build with system glibc
#define __lava_cplusplus __cplusplus
#undef __cplusplus
#include <string.h>
#define __cplusplus __lava_cplusplus


#include <string>
#include <memory>
#include <limits>

// void* operator new(std::size_t n) {
//   void * const p = std::malloc(n);
//   return p;
// }

// void operator delete(void * p) noexcept { std::free(p); }

namespace __lava
{
    using bitwidth_t = uint8_t;

    struct construct_shared_t {};
    static constexpr construct_shared_t construct_shared; 

    template< typename domain >
    struct domain_ref : domain
    {
        using base = typename domain::base;

        domain_ref( const domain_ref &v ) : domain( v.unsafe_ptr(), construct_shared ) {}
        domain_ref( const domain     &v ) : domain( v.unsafe_ptr(), construct_shared ) {}
        domain_ref( const base       &v ) : domain( v.unsafe_ptr(), construct_shared ) {}
        explicit domain_ref( void    *v ) : domain( v             , construct_shared ) {}

        ~domain_ref() { this->disown(); }
    };

    using i1  = bool;
    using i8  = uint8_t;
    using i16 = uint16_t;
    using i32 = uint32_t;
    using i64 = uint64_t;

    using si8  = int8_t;
    using si16 = int16_t;
    using si32 = int32_t;
    using si64 = int64_t;

    using f32 = float;
    using f64 = double;
    struct array_ref { void *base; size_t size; };

    template< typename type >
    constexpr uint8_t bitwidth_v = std::is_same_v< type, bool > ? 1 : sizeof( type ) * 8;

    template< typename T > struct number_type { using type = T; };
    template< bool, int > struct number_;

    template<> struct number_< true, 1 >  : number_type< bool > {};
    template<> struct number_< true, 8 >  : number_type< int8_t > {};
    template<> struct number_< true, 16 > : number_type< int16_t > {};
    template<> struct number_< true, 32 > : number_type< int32_t > {};
    template<> struct number_< true, 64 > : number_type< int64_t > {};

    template<> struct number_< false, 1 >  : number_type< bool > {};
    template<> struct number_< false, 8 >  : number_type< uint8_t > {};
    template<> struct number_< false, 16 > : number_type< uint16_t > {};
    template<> struct number_< false, 32 > : number_type< uint32_t > {};
    template<> struct number_< false, 64 > : number_type< uint64_t > {};

    template< bool s, int w >
    using number = typename number_< s, w >::type;

    template< bool, bool, int > struct value_;

    template< bool s, int w >
    struct value_< false /* non-pointer */, s, w > : number_< s, w > {};

    template< bool s, int w >
    struct value_< true /* pointer */, s, w >
    {
        using type = std::add_pointer_t< number< s, w > >;
    };

    template< bool p, bool s, int w >
    using value = typename value_< p, s, w >::type;

    template< bool s, int w, typename t >
    static value< std::is_pointer_v< t >, s, w > cast( t v )
    {
        return static_cast< decltype( cast< s, w >( v ) ) >( v );
    }

    template< bool signedness, typename fn_t, typename... args_t >
    auto call( bitwidth_t bw, fn_t fn, args_t&&... args )
    {
        switch ( bw ) {
            case 1 : return fn( cast< signedness,  1 >( std::forward< args_t >( args ) )...  );
            case 8 : return fn( cast< signedness,  8 >( std::forward< args_t >( args ) )...  );
            case 16: return fn( cast< signedness, 16 >( std::forward< args_t >( args ) )...  );
            case 32: return fn( cast< signedness, 32 >( std::forward< args_t >( args ) )...  );
            case 64: return fn( cast< signedness, 64 >( std::forward< args_t >( args ) )...  );
        }
        __builtin_unreachable();
    }

    static constexpr auto callu = []( const auto & ... xs ) { return call< false >( xs... ); };
    static constexpr auto calls = []( const auto & ... xs ) { return call< true  >( xs... ); };

    template< typename self_t >
    struct domain_mixin
    {
        using st = self_t;
        using sr = const self_t &;
        using bw = bitwidth_t;

        using index_dom = self_t;
        using scalar_dom = self_t;

        [[noreturn]] static inline st fail( const char *msg = "" )
        {
            fprintf( stderr, "[lamp fail]: unsupported %s\n", msg );
            __builtin_trap();
        }

        self_t &      self()       { return *static_cast< self_t* >( this ); }
        const self_t &self() const { return *static_cast< const self_t* >( this ); }

        st clone() const { return { self().get() }; }

        static st lift_i1( i1 v )   { return st::lift( v ); }
        static st lift_i8( i8 v )   { return st::lift( v ); }
        static st lift_i16( i16 v ) { return st::lift( v ); }
        static st lift_i32( i32 v ) { return st::lift( v ); }
        static st lift_i64( i64 v ) { return st::lift( v ); }

        static st lift_si8( si8 v )   { return st::lift( v ); }
        static st lift_si16( si16 v ) { return st::lift( v ); }
        static st lift_si32( si32 v ) { return st::lift( v ); }
        static st lift_si64( si64 v ) { return st::lift( v ); }

        static st lift_f32( f32 v ) { return st::lift( v ); }
        static st lift_f64( f64 v ) { return st::lift( v ); }

        static st lift_arr( void *v, unsigned s ) { return st::lift( array_ref{ v, s } ); }
        static st lift_ptr( void *v ) { return st::lift( v ); }

        static st lower( sr ) { return fail( "lower" ); }

        template< typename size >
        static st op_alloca( const size&, bw ) { return fail( "alloca" ); }

        static st op_join( sr, sr ) { return fail( "join" ); }
        static st op_meet( sr, sr ) { return fail( "meet" ); }

        static st op_not ( sr ) { return fail( "not" ); }
        static st op_neg ( sr ) { return fail( "neg" ); }

        static st op_add ( sr, sr ) { return fail( "add" ); }
        static st op_sub ( sr, sr ) { return fail( "sub" ); }
        static st op_mul ( sr, sr ) { return fail( "mul" ); }
        static st op_sdiv( sr, sr ) { return fail( "sdiv" ); }
        static st op_udiv( sr, sr ) { return fail( "udiv" ); }
        static st op_srem( sr, sr ) { return fail( "srem" ); }
        static st op_urem( sr, sr ) { return fail( "urem" ); }

        static st op_fadd( sr, sr ) { return fail( "fadd" ); }
        static st op_fsub( sr, sr ) { return fail( "fsub" ); }
        static st op_fmul( sr, sr ) { return fail( "fmul" ); }
        static st op_fdiv( sr, sr ) { return fail( "fdiv" ); }
        static st op_frem( sr, sr ) { return fail( "frem" ); }

        static st op_shl ( sr, sr ) { return fail( "shl"  ); }
        static st op_ashr( sr, sr ) { return fail( "ashr" ); }
        static st op_lshr( sr, sr ) { return fail( "lshr" ); }
        static st op_and ( sr, sr ) { return fail( "and"  ); }
        static st op_or  ( sr, sr ) { return fail( "or"   ); }
        static st op_xor ( sr, sr ) { return fail( "xor"  ); }

        static st op_eq ( sr, sr ) { return fail( "eq" ); }
        static st op_ne ( sr, sr ) { return fail( "ne" ); }
        static st op_ugt( sr, sr ) { return fail( "ugt" ); }
        static st op_uge( sr, sr ) { return fail( "uge" ); }
        static st op_ult( sr, sr ) { return fail( "ult" ); }
        static st op_ule( sr, sr ) { return fail( "ule" ); }
        static st op_sgt( sr, sr ) { return fail( "sgt" ); }
        static st op_sge( sr, sr ) { return fail( "sge" ); }
        static st op_slt( sr, sr ) { return fail( "slt" ); }
        static st op_sle( sr, sr ) { return fail( "sle" ); }

        static st op_foeq( sr, sr ) { return fail( "foeq" ); }
        static st op_fogt( sr, sr ) { return fail( "fogt" ); }
        static st op_foge( sr, sr ) { return fail( "foge" ); }
        static st op_folt( sr, sr ) { return fail( "folt" ); }
        static st op_fole( sr, sr ) { return fail( "fole" ); }
        static st op_fone( sr, sr ) { return fail( "fone" ); }
        static st op_ford( sr, sr ) { return fail( "ford" ); }
        static st op_funo( sr, sr ) { return fail( "funo" ); }
        static st op_fueq( sr, sr ) { return fail( "fueq" ); }
        static st op_fugt( sr, sr ) { return fail( "fugt" ); }
        static st op_fuge( sr, sr ) { return fail( "fuge" ); }
        static st op_fult( sr, sr ) { return fail( "fult" ); }
        static st op_fule( sr, sr ) { return fail( "fule" ); }
        static st op_fune( sr, sr ) { return fail( "fune" ); }

        static st op_ffalse( sr, sr ) { return fail( "ffalse" ); }
        static st op_ftrue ( sr, sr ) { return fail( "ftrue" ); }

        static st op_trunc  ( sr, bw ) { return fail( "trunc"   ); }
        static st op_fptrunc( sr, bw ) { return fail( "fptrunc" ); }
        static st op_sitofp ( sr, bw ) { return fail( "sitofp"  ); }
        static st op_uitofp ( sr, bw ) { return fail( "uitofp"  ); }
        static st op_zext   ( sr, bw ) { return fail( "zext"    ); }
        static st op_zfit   ( sr, bw ) { return fail( "zfit"    ); }
        static st op_sext   ( sr, bw ) { return fail( "sext"    ); }
        static st op_fpext  ( sr, bw ) { return fail( "fpext"   ); }
        static st op_fptosi ( sr, bw ) { return fail( "fptosi"  ); }
        static st op_fptoui ( sr, bw ) { return fail( "fptoui"  ); }

        static st op_concat ( sr, sr ) { return fail( "concat" ); }
        static st op_extract( sr, bw, bw ) { return fail( "extract" ); }

        template< typename scal > static void op_store( sr, const scal&, bw ) { fail( "store" ); }
        
        static st op_load( sr, bw ) { return fail( "load" ); }

        static void op_dealloca( sr s ) { s.~self_t(); }

        static void dump( sr ) { printf( "dump not implemented" ); }
        static std::string trace( sr ) { return "trace is not implemented"; }
        
        static void report( const char * op )
        { 
            fprintf( stderr, "[lamp warning]: unsupported bop %s\n", op );
        }

        // backward operations
        static void bop_not ( sr, sr ) { report( "not" ); }
        static void bop_neg ( sr, sr ) { report( "neg" ); }

        static void bop_add ( sr, sr, sr ) { report( "add" ); }
        static void bop_sub ( sr, sr, sr ) { report( "sub" ); }
        static void bop_mul ( sr, sr, sr ) { report( "mul" ); }
        static void bop_sdiv( sr, sr, sr ) { report( "sdiv" ); }
        static void bop_udiv( sr, sr, sr ) { report( "udiv" ); }
        static void bop_srem( sr, sr, sr ) { report( "srem" ); }
        static void bop_urem( sr, sr, sr ) { report( "urem" ); }

        static void bop_fadd( sr, sr, sr ) { report( "fadd" ); }
        static void bop_fsub( sr, sr, sr ) { report( "fsub" ); }
        static void bop_fmul( sr, sr, sr ) { report( "fmul" ); }
        static void bop_fdiv( sr, sr, sr ) { report( "fdiv" ); }
        static void bop_frem( sr, sr, sr ) { report( "frem" ); }

        static void bop_shl ( sr, sr, sr ) { report( "shl" ); }
        static void bop_ashr( sr, sr, sr ) { report( "ashr" ); }
        static void bop_lshr( sr, sr, sr ) { report( "lshr" ); }
        static void bop_and ( sr, sr, sr ) { report( "and" ); }
        static void bop_or  ( sr, sr, sr ) { report( "or" ); }
        static void bop_xor ( sr, sr, sr ) { report( "xor" ); }

        static void bop_eq ( sr, sr, sr ) { report( "eq" ); }
        static void bop_ne ( sr, sr, sr ) { report( "ne" ); }
        static void bop_ugt( sr, sr, sr ) { report( "ugt" ); }
        static void bop_uge( sr, sr, sr ) { report( "uge" ); }
        static void bop_ult( sr, sr, sr ) { report( "ult" ); }
        static void bop_ule( sr, sr, sr ) { report( "ule" ); }
        static void bop_sgt( sr, sr, sr ) { report( "sgt" ); }
        static void bop_sge( sr, sr, sr ) { report( "sge" ); }
        static void bop_slt( sr, sr, sr ) { report( "slt" ); }
        static void bop_sle( sr, sr, sr ) { report( "sle" ); }

        static void bop_foeq( sr, sr, sr ) { report( "foeq" ); }
        static void bop_fogt( sr, sr, sr ) { report( "fogt" ); }
        static void bop_foge( sr, sr, sr ) { report( "foge" ); }
        static void bop_folt( sr, sr, sr ) { report( "folt" ); }
        static void bop_fole( sr, sr, sr ) { report( "fole" ); }
        static void bop_fone( sr, sr, sr ) { report( "fone" ); }
        static void bop_ford( sr, sr, sr ) { report( "ford" ); }
        static void bop_funo( sr, sr, sr ) { report( "funo" ); }
        static void bop_fueq( sr, sr, sr ) { report( "fueq" ); }
        static void bop_fugt( sr, sr, sr ) { report( "fugt" ); }
        static void bop_fuge( sr, sr, sr ) { report( "fuge" ); }
        static void bop_fult( sr, sr, sr ) { report( "fult" ); }
        static void bop_fule( sr, sr, sr ) { report( "fule" ); }
        static void bop_fune( sr, sr, sr ) { report( "fune" ); }

        static void bop_ffalse( sr, sr, sr ) { report( "ffalse" ); }
        static void bop_ftrue ( sr, sr, sr ) { report( "ftrue" ); }

        static void bop_trunc  ( sr, sr ) { report( "trunc" ); }
        static void bop_fptrunc( sr, sr ) { report( "fptrunc" ); }
        static void bop_sitofp ( sr, sr ) { report( "sitofp" ); }
        static void bop_uitofp ( sr, sr ) { report( "uitofp" ); }
        static void bop_zext   ( sr, sr ) { report( "zext" ); }
        static void bop_zfit   ( sr, sr ) { report( "zfit" ); }
        static void bop_sext   ( sr, sr ) { report( "sext" ); }
        static void bop_fpext  ( sr, sr ) { report( "fpext" ); }
        static void bop_fptosi ( sr, sr ) { report( "fptosi" ); }
        static void bop_fptoui ( sr, sr ) { report( "fptoui" ); }

        static void bop_concat ( sr, sr, sr ) { report( "concat" ); }
    };

    struct base {};

} // namespace __lava
