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

        pointer_data( void *p, arithmetic &&n )
            : ptr( p ), num( std::move( n ) )
        {}

        ~pointer_data() {
            printf("destroy data\n");
        }

        [[nodiscard]] raw_t raw() const { return raw_t( ptr ); }

        void *ptr;
        arithmetic num;
    };

    template< typename arithmetic >
    using pointer_storage = tagged_storage< pointer_data< arithmetic > >;


    template< typename arithmetic >
    struct pointer_arith : pointer_storage< arithmetic >
                         , domain_mixin< pointer_arith< arithmetic > >
    {
        using base = domain_mixin< pointer_arith >;

        using storage = pointer_storage< arithmetic >;
        using storage::storage;

        using av = arithmetic;
        using aref = scalar_domain_ref< av >;

        using pv = pointer_arith;
        using pr = const pointer_arith &;

        using bw = typename base::bw;
        using ptr = void*;

        template< typename type > static pv any() { return base::fail(); }

        static pv lift( const constant& con ) { return constant::lift_to< pointer_arith >( con ); }
        static pv lift( const arithmetic&   ) { return base::fail(); }

        template< typename type >
        static pv lift( const type &value )
        {
            void * ptr = [&value] {
                if constexpr ( std::is_integral_v< type > )
                    return reinterpret_cast< void * >( uintptr_t( value ) );
                else
                    return reinterpret_cast< void * >( value );
            } ();

            printf( "--- lift value %p\n", ptr );

            if ( alive( ptr ) )
                base::fail();
            //    return lift_objid( ptr );

            auto lifted = av::lift( value );
            printf( "lifted %p\n", lifted.unsafe_ptr() );
            return { ptr, std::move( lifted ) };
        }

        static pv lift_objid( void *ptr )
        {
            printf( "--- lift pointer %p\n", ptr );
            auto vp = pointer_split( ptr );

            if ( !vp.obj )
                return { ptr, av::lift( uintptr_t( ptr ) ) };

            aref obj = abstract_objid( vp.obj );

            // auto obj = lift_objid( vp.obj );
            // auto off = as::lift( vp.off );

            //auto cl = obj.clone();
            return { ptr, aref( obj.clone().disown() ) };//as::concat( obj, off ) };
        }

        static aref abstract_objid( uint32_t obj )
        {
            auto st = state();

            auto abstract = av::template any< uintptr_t >();

            // ensure that abstract objid is unique and nonnull
            // as::assume( abstract != av::lift( uintptr_t( 0 ) ) );
            // for ( auto [key, val] : st->map )
            //    as::assume( abstract != val );

            st->map.emplace( obj, abstract );
            return aref( abstract.disown() );
        }

        static constant lower( pr ) { __builtin_unreachable(); };

        static void assume( pr, bool ) {}

        static tristate to_tristate( pr ) { return maybe; }

        static bool alive( uint32_t obj ) { return obj && state()->map.count( obj ); }
        static bool alive( void * ptr ) { return alive( pointer_split( ptr ).obj ); }

        template< typename op_t, typename ...args_t >
        static pv apply( op_t op, args_t... args )
        {
            return { ptr( op( args->raw() ... ) ), op( aref( args->num ) ... ) };
        }

        /*template< typename op_t >
        static arithmetic cmp( op_t op, pr a, pr b )
        {
            return op( aref( a->num ), aref( b->num ) );
        }

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
        }*/

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

        /*static constexpr auto shl = [] ( const auto &a, const auto &b ) { return a << b; };
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
        static pv op_zfit(  pr p, bw w ) { return cast( p, w, av::op_zfit ); }*/

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
