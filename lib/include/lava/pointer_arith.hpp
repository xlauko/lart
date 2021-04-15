/*
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

#pragma once

#include <lava/support/tristate.hpp>
#include <lava/support/base.hpp>
#include <lava/support/pointer.hpp>
#include <lava/support/reference.hpp>
#include <lava/constant.hpp>

#include <runtime/map.hpp>

namespace __lava
{
    extern "C" void *__pointers_state;

    template< typename arithmetic >
    struct [[gnu::packed]] pointer_data
    {
        using raw_t = uintptr_t;

        [[nodiscard]] raw_t raw() const { return raw_t( ptr ); }

        pointer_data( const pointer_data &o )
            : pointer_data( o.ptr, o.num.clone() )
        {}

        pointer_data( void *ptr, arithmetic &&n )
            : ptr( ptr ), num( std::move( n ) )
        {}

        void *ptr;
        arithmetic num;
    };

    template< template< typename > typename storage, typename arithmetic >
    struct pointer_arith : storage< pointer_data< arithmetic > >
                         , domain_mixin< pointer_arith< storage, arithmetic > >
    {
        using base = storage< pointer_data< arithmetic > >;
        using mixin = domain_mixin< pointer_arith >;

        using bw = typename mixin::bw;
        using base::base;

        using av = arithmetic;
        using ar = const arithmetic &;
        
        using pv = pointer_arith;
        using pr = const pointer_arith &;
        
        using ref = domain_ref< pv >;
        using aref = scalar_domain_ref< av >;

        using ptr = void*;

        template< typename type > static pv any() { return mixin::fail(); }

        static pv lift( const constant< storage >& con )
        {
            return constant< storage >::template lift_to< pointer_arith >( con );
        }
        static pv lift( const arithmetic&   ) { return mixin::fail(); }

        template< typename type >
        static pv lift( const type &value )
        {
            void * ptr = [&value] {
                if constexpr ( std::is_integral_v< type > )
                    return reinterpret_cast< void * >( uintptr_t( value ) );
                else
                    return reinterpret_cast< void * >( value );
            } ();

            if ( alive( ptr ) )
                mixin::fail();
            //    return lift_objid( ptr );

            return { ptr, av::lift( value ) };
        }

        static void assume_add_no_overflow( ar a, ar b )
        {
            auto max = av::lift( std::numeric_limits< uintptr_t >::max() );
            auto diff = aref( max ) - aref( b );
            aref::assume( aref( a ) < aref( diff ) );
        }

        static pv lift_objid( void *ptr )
        {
            auto vp = pointer_split( ptr );

            if ( !vp.obj )
                return { ptr, av::lift( uintptr_t( ptr ) ) };

            auto obj = abstract_objid( vp.obj );
            auto off = av::lift( uintptr_t( vp.off ) );
            
            assume_add_no_overflow( obj, off );
            return { ptr, av::op_add( obj, off ) };
        }

        static aref abstract_objid( uint32_t obj )
        {
            auto st = state();

            if ( auto it = st->map.find( obj ); it != st->map.end() ) {
                return it->second;
            }
            
            auto abstract = av::template any< uintptr_t >();
            auto null     = av::lift( uintptr_t( 0 ) );
            
            // ensure that abstract objid is unique and nonnull
            aref::assume( aref( abstract ) != aref( null ) );
            for ( auto &[key, val] : st->map )
                aref::assume( aref( abstract ) != aref( val ) );

            auto it = st->map.emplace( obj, std::move( abstract ) ).first;
            return it->second;
        }

        static void assume( pr, bool ) { mixin::fail(); }

        static tristate to_tristate( pr ) { return maybe; }

        static bool alive( uint32_t obj ) { return obj && state()->map.count( obj ); }
        static bool alive( void * ptr ) { return alive( pointer_split( ptr ).obj ); }

        template< typename op_t, typename ...args_t >
        static pv apply( op_t op, const args_t &... args )
        {
            return { ptr( op( args->raw() ... ) ), op( aref( args->num ) ... ) };
        }

        template< typename op_t >
        static av cmp( op_t op, pr a, pr b ) { return op( aref( a->num ), aref( b->num ) ); }

        static uintptr_t mask( bw w )
        {
            return w ? ( ( 1ull << ( w - 1 ) ) | mask( w - 1 ) ) : 0;
        }

        static void * set_bw( uintptr_t p, bw w )
        {
            return ptr( p & mask( w ) );
        }

        template< typename op_t >
        static pv cast( pr p, bw w, op_t op )
        {
            return { set_bw( p->raw(), w ), op( p->num, w ) };
        }

        static pv op_add ( pr a, pr b ) { return apply( std::plus(), a, b ); }
        static pv op_sub ( pr a, pr b ) { return apply( std::minus(), a, b ); }
        static pv op_mul ( pr a, pr b ) { return apply( std::multiplies(), a, b ); }
        static pv op_sdiv( pr a, pr b ) { return apply( std::divides(), a, b ); }
        static pv op_udiv( pr a, pr b ) { return apply( std::divides(), a, b ); }
        static pv op_srem( pr a, pr b ) { return apply( std::modulus(), a, b ); }
        static pv op_urem( pr a, pr b ) { return apply( std::modulus(), a, b ); }

        static pv op_and( pr a, pr b ) { return apply( std::bit_and(), a, b ); }
        static pv op_or ( pr a, pr b ) { return apply( std::bit_or(), a, b ); }
        static pv op_xor( pr a, pr b ) { return apply( std::bit_xor(), a, b ); }

        static constexpr auto shl = [] ( const auto &a, const auto &b ) { return a << b; };
        static constexpr auto shr = [] ( const auto &a, const auto &b ) { return a >> b; };

        static pv op_shl ( pr a, pr b ) { return apply( shl, a, b ); }
        static pv op_lshr( pr a, pr b ) { return apply( shr, a, b ); }

        static av op_eq ( pr a, pr b ) { return cmp( std::equal_to(), a, b ); }
        static av op_ne ( pr a, pr b ) { return cmp( std::not_equal_to(), a, b ); }
        static av op_ult( pr a, pr b ) { return cmp( std::less(), a, b ); }
        static av op_ugt( pr a, pr b ) { return cmp( std::greater(), a, b ); }
        static av op_ule( pr a, pr b ) { return cmp( std::less_equal(), a, b ); }
        static av op_uge( pr a, pr b ) { return cmp( std::greater_equal(), a, b ); }

        static pv op_trunc( pr p, bw w ) { return cast( p, w, av::op_trunc ); }
        static pv op_zext(  pr p, bw w ) { return cast( p, w, av::op_zext ); }
        static pv op_zfit(  pr p, bw w ) { return cast( p, w, av::op_zfit ); }

        static constexpr auto store = [] ( auto p, auto v ) { *p = v; };

        template< typename val >
        static void op_store( pr p, const val &v, bitwidth_t bw )
        {
            if ( !p->ptr )
                mixin::fail( "null pointer dereference" );

            if constexpr ( std::is_same_v< val, constant< storage > > )
            {
                callu( bw, store, p->ptr, v->value );
            }
            else if constexpr( std::is_same_v< val, pointer_arith > )
            {
                callu( bw, store, p->ptr, v->raw() );
            }
            else
            {
                fprintf( stderr, "unsupported store %s", __PRETTY_FUNCTION__ );
                __builtin_trap();
            }
        }


        struct state_t
        {
            using objid = uint32_t;
            using pointer = arithmetic;

            void erase( void * ptr )
            {
                map.erase( pointer_split( ptr ).obj );
            }

            __lart::rt::array_map< objid, arithmetic > map; /* maps objids to abstract representations */
        };

        static state_t * state() { return static_cast< state_t * >( __pointers_state ); }
    };

} // namespace __lava
