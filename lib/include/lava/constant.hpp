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

#include <lava/support/tristate.hpp>
#include <lava/support/scalar.hpp>
#include <lava/support/base.hpp>

#include <cassert>

namespace __lava
{
    struct [[gnu::packed]] constant_data
    {
        uint64_t value;
        enum type_t { bv, fp, ptr } type;
        bitwidth_t bw;
    };

    using constant_storage = tagged_storage< constant_data >;

    struct constant : constant_storage, domain_mixin< constant >
    {
        using base = domain_mixin< constant >;
        using sc = scalar< constant >;

        constant() = default;
        constant( void *v, construct_shared_t s ) : constant_storage( v, s ) {}
        constant( constant_data::type_t t, bitwidth_t b, uint64_t v )
            : tagged_storage< constant_data >( constant_data{ v, t, b } )
        {}

        template< typename concrete_t >
        static auto lift( concrete_t value )
            -> std::enable_if_t< std::is_trivial_v< concrete_t >, constant >
        {
            static_assert( sizeof( concrete_t ) <= 8 );

            uint64_t val;
            constant_data::type_t type;

            if constexpr ( std::is_pointer_v< concrete_t > )
            {
                val = reinterpret_cast< uintptr_t >( value );
                type = constant_data::ptr;
            }
            else
                val = value;

            if constexpr ( std::is_integral_v< concrete_t > )
                type = constant_data::bv;
            if constexpr ( std::is_floating_point_v< concrete_t > )
                type = constant_data::fp;

            return { type, bitwidth_v< concrete_t >, val };
        }

        static constant lift( array_ref ) { __builtin_unreachable(); }

        static constant lower( const constant &c ) { return c; }

        template< typename T >
        static T lift_to( const constant & con )
        {
            auto lift = [] ( auto v ) { return T::lift( v ); };
            return callu( con->bw, lift, con->value );
        }

        template< typename > static constant any()
        {
            __builtin_unreachable();
            // UNREACHABLE( "Constant domain does not provide 'any' operation" );
        }

        template< bool signedness, typename F, typename... val_t  >
        static constant with_type( F f_, const val_t & ... vals )
        {
            bitwidth_t bw = std::max( { vals->bw ... } );
            assert( ( ( bw == vals->bw ) && ... ) );

            auto f = [&]( const auto & ... x ) -> uint64_t { return static_cast< uint64_t >( f_( x... ) ); };
            auto unwrap = [] ( const constant &v ) -> uint64_t { return static_cast< uint64_t >( v->value ); };

            if ( ( ( vals->type == constant_data::fp ) || ... ) )
                base::fail();

            bool pt = ( ( ( vals->type == constant_data::ptr ) || ... ) );
            auto type = pt ? constant_data::ptr : constant_data::bv;
            return { type, bw, call< signedness >( bw, f, unwrap( vals )... ) };
        }

        static constexpr auto wtu = []( const auto & ... xs ) { return with_type< false >( xs... ); };
        static constexpr auto wts = []( const auto & ... xs ) { return with_type< true  >( xs... ); };

        static uint64_t mask( bitwidth_t bw )
        {
            return bw ? ( ( 1ull << ( bw - 1 ) ) | mask( bw - 1 ) ) : 0;
        }

        static constant set_bw( const constant &c, bitwidth_t bw )
        {
            auto rv = c;
            rv->bw = bw;
            rv->value &= mask( bw );
            return rv;
        }

        static tristate to_tristate( const constant &val )
        {
            return tristate( static_cast< bool >( val->value ) );
        }

        static constant assume( const constant &c, bool expect )
        {
            if ( c->value != expect )
                __lart_cancel();
            return c;
        }

        using cv = constant;
        using cr = const constant &;

        static cv op_not ( cr a ) { return wtu( std::logical_not(), a ); }
        static cv op_neg ( cr a ) { return wtu( std::bit_not(), a ); }

        static cv op_add ( cr a, cr b ) { return wtu( std::plus(), a, b ); }
        static cv op_sub ( cr a, cr b ) { return wtu( std::minus(), a, b ); }
        static cv op_mul ( cr a, cr b ) { return wtu( std::multiplies(), a, b ); }
        static cv op_sdiv( cr a, cr b ) { return wts( std::divides(), a, b ); }
        static cv op_udiv( cr a, cr b ) { return wtu( std::divides(), a, b ); }
        static cv op_srem( cr a, cr b ) { return wts( std::modulus(), a, b ); }
        static cv op_urem( cr a, cr b ) { return wtu( std::modulus(), a, b ); }

        static cv op_and( cr a, cr b ) { return wtu( std::bit_and(), a, b ); }
        static cv op_or ( cr a, cr b ) { return wtu( std::bit_or(), a, b ); }
        static cv op_xor( cr a, cr b ) { return wtu( std::bit_xor(), a, b ); }

        static constexpr auto shl = []( auto a, auto b ) { return a << b; };
        static constexpr auto shr = []( auto a, auto b ) { return a >> b; };

        static cv op_shl ( cr a, cr b ) { return wtu( shl, a, b ); }
        static cv op_ashr( cr a, cr b ) { return wts( shr, a, b ); }
        static cv op_lshr( cr a, cr b ) { return wtu( shr, a, b ); }

        static cv op_eq ( cr a, cr b ) { return wtu( std::equal_to(), a, b ); }
        static cv op_ne ( cr a, cr b ) { return wtu( std::not_equal_to(), a, b ); }
        static cv op_ult( cr a, cr b ) { return wtu( std::less(), a, b ); }
        static cv op_ugt( cr a, cr b ) { return wtu( std::greater(), a, b ); }
        static cv op_ule( cr a, cr b ) { return wtu( std::less_equal(), a, b ); }
        static cv op_uge( cr a, cr b ) { return wtu( std::greater_equal(), a, b ); }

        static cv op_slt( cr a, cr b ) { return wts( std::less(), a, b ); }
        static cv op_sgt( cr a, cr b ) { return wts( std::greater(), a, b ); }
        static cv op_sle( cr a, cr b ) { return wts( std::less_equal(), a, b ); }
        static cv op_sge( cr a, cr b ) { return wts( std::greater_equal(), a, b ); }

        static cv op_trunc( cv c, bw w ) { return set_bw( c, w ); }
        static cv op_zext( cv c, bw w )  { return set_bw( c, w ); }
        static cv op_zfit( cv c, bw w )  { return set_bw( c, w ); }
        static cv op_sext( cv c, bw w )
        {
            return set_bw( wts( []( auto v ) { return int64_t( v ); }, c ), w );
        }

        static cv op_concat( cv a, cv b )
        {
            auto bw = lift( a->bw );
            a = op_zext( a, a->bw + b->bw );
            auto r = op_or( op_shl( a, bw ), b );
            return r;
        }

        static void * pointer( cr c )
        {
            return reinterpret_cast< void * >( static_cast< uintptr_t >( c->value ) );
        }

        static constexpr auto load = [] ( auto * p ) { return lift( *p ); };
        static constexpr auto store = [] ( auto * p, auto v ) { *p = v; };

        static cv op_load( cr p, bitwidth_t bw ) { return callu( bw, load, pointer( p ) ); }

        template< typename val >
        static cv op_store( cr p, val v, bitwidth_t bw )
        {
            if constexpr ( std::is_same_v< val, constant > )
            {
                callu( bw, store, pointer( p ), v->value );
                return p;
            }
            else
            {
                // __dios_trace_f( "unsupported store %s", __PRETTY_FUNCTION__ );
                __builtin_trap();
            }
        }
    };

    static_assert( sizeof( constant ) == 8 );

} // namespace __lava