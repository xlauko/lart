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

#pragma once

#include <lava/support/base.hpp>
#include <lava/support/tristate.hpp>
#include <lava/support/operations.hpp>

#include <type_traits>

namespace __lava
{
    template< typename A, typename B >
    struct /* [[gnu::packed]] */ product_storage
    {
        constexpr product_storage(A &&a, B &&b)
            : first(std::move(a)), second(std::move(b))
        {}

        A first;
        B second;
    };

    struct lower_first {};
    struct lower_second {};
    struct lower_disabled {};

    struct to_tristate_first {};
    struct to_tristate_second {};
    struct to_tristate_disabled {};

    template<
        typename lowering_strategy_t,
        typename to_tristate_strategy_t
    >
    struct product_config
    {
        using lowering_strategy = lowering_strategy_t;
        using to_tristate_strategy = to_tristate_strategy_t;
    };

    using default_product_config = product_config<
        lower_disabled,
        to_tristate_disabled
    >;

    template<
        typename A, typename B,
        template< typename > typename storage,
        typename product_config = default_product_config
    >
    struct product : storage< product_storage< A, B > >
                   , domain_mixin< product< A, B, storage, product_config > >
    {
        using base = storage< product_storage< A, B > >;
        using mixin = domain_mixin< product >;

        using config = product_config;

        using bw = typename mixin::bw;
        using base::base;

        using pv = product;
        using pr = const product &;

        using left_type = A;
        using right_type = B;


        const auto &self() const
        {
            return static_cast< const product& >( *this );
        }

        const left_type& left()  const { return self()->first; }
        const right_type& right() const { return self()->second; }

        // product( void * p, __dios::construct_shared_t s ) : storage( p, s ) {}
        // product( const A &a, const B &b ) : storage( data{ a, b } ) {}
        // product( A &&a, B &&b ) : storage( data{ std::forward< A >( a ), std::forward< B >( b ) } ) {}

        product clone() const
        {
            return product( left().clone(), right().clone() );
        }

        static tristate to_tristate( pr v )
        {
            if constexpr ( std::is_same_v< typename config::to_tristate_strategy, to_tristate_first > )
                return A::to_tristate( v->first );
            if constexpr ( std::is_same_v< typename config::to_tristate_strategy, to_tristate_second > )
                return B::to_tristate( v->second );
            mixin::fail( "to_tristate is disabled" );
        }

        static constexpr auto wrap = __lava::op::wrap;

        template< typename op_t >
        static constexpr pv bin( op_t op, pr a, pr b )
        {
            return { op( a->first, b->first ), op( a->second, b->second ) };
        }

        template< typename op_t >
        static constexpr void bin_bop( op_t op, pr r, pr a, pr b )
        {
            op( r->first, a->first, b->first );
            op( r->second, a->second, b->second );
        }

        template< typename op_t >
        static constexpr pv un( op_t op, pr a )
        {
            return { op( a->first ), op( a->second ) };
        }

        template< typename op_t >
        static constexpr void un_bop( op_t op, pr r, pr a )
        {
            op( r->first, a->first );
            op( r->second, a->second );
        }


        // static pv lift( const constant& con ) { return constant::lift_to< product >( con ); }

        template< typename type > static pv lift( const type &value )
        {
            return { A::lift( value ), B::lift( value ) };
        }

        // template< typename X, typename Y, typename orig_config >
        // static pv lift( const product< X, Y, orig_config > &p )
        // {
        //     return { A::lift( p->first ), B::lift( p->second ) };
        // }

        template< typename type > static pv any()
        {
            return { A::template any< type >(), B::template any< type >() };
        }

        template< typename type > static pv any(const variadic_list &args)
        {
            return { A::template any< type >(args), B::template any< type >(args) };
        }

        template< typename type > static pv any(type from, type to)
        {
            return { A::any(from, to), B::any(from, to) };
        }

        // template< typename op_t >
        // static constant lower( op_t op, pr v )
        // {
        //     if constexpr ( std::is_same_v< lowering_strategy, lower_first > )
        //         return op( v->first );
        //     if constexpr ( std::is_same_v< lowering_strategy, lower_second > )
        //         return op( v->second );
        //     base::fail( "lowering disabled" );
        // }

        // static constant lower ( pr v ) { return lower( wrap( op::lower ) , v ); }

        template< typename size >
        static pv op_alloca( const size &s, bw b )
        {
            return un( wrap( op::_alloca, b ), s );
        }

        static pv op_join( pr a, pr b ) { return bin( wrap( op::join ), a, b ); }
        static pv op_meet( pr a, pr b ) { return bin( wrap( op::meet ), a, b ); }

        static pv op_add ( pr a, pr b ) { return bin( wrap( op::add ), a, b ); }
        static pv op_sub ( pr a, pr b ) { return bin( wrap( op::sub ), a, b ); }
        static pv op_mul ( pr a, pr b ) { return bin( wrap( op::mul ), a, b ); }

        static pv op_sdiv( pr a, pr b ) { return bin( wrap( op::sdiv ), a, b ); }
        static pv op_udiv( pr a, pr b ) { return bin( wrap( op::udiv ), a, b ); }
        static pv op_srem( pr a, pr b ) { return bin( wrap( op::srem ), a, b ); }
        static pv op_urem( pr a, pr b ) { return bin( wrap( op::urem ), a, b ); }

        static pv op_fadd ( pr a, pr b ) { return bin( wrap( op::fadd ), a, b ); }
        static pv op_fsub ( pr a, pr b ) { return bin( wrap( op::fsub ), a, b ); }
        static pv op_fmul ( pr a, pr b ) { return bin( wrap( op::fmul ), a, b ); }
        static pv op_fdiv ( pr a, pr b ) { return bin( wrap( op::fdiv ), a, b ); }
        static pv op_frem ( pr a, pr b ) { return bin( wrap( op::frem ), a, b ); }

        static pv op_shl ( pr a, pr b ) { return bin( wrap( op::shl  ), a, b ); }
        static pv op_ashr( pr a, pr b ) { return bin( wrap( op::ashr ), a, b ); }
        static pv op_lshr( pr a, pr b ) { return bin( wrap( op::lshr ), a, b ); }
        static pv op_and ( pr a, pr b ) { return bin( wrap( op::_and ), a, b ); }
        static pv op_or  ( pr a, pr b ) { return bin( wrap( op::_or  ), a, b ); }
        static pv op_xor ( pr a, pr b ) { return bin( wrap( op::_xor ), a, b ); }

        static pv op_eq ( pr a, pr b ) { return bin( wrap( op::eq  ), a, b ); }
        static pv op_ne ( pr a, pr b ) { return bin( wrap( op::ne  ), a, b ); }
        static pv op_ugt( pr a, pr b ) { return bin( wrap( op::ugt ), a, b ); }
        static pv op_uge( pr a, pr b ) { return bin( wrap( op::uge ), a, b ); }
        static pv op_ult( pr a, pr b ) { return bin( wrap( op::ult ), a, b ); }
        static pv op_ule( pr a, pr b ) { return bin( wrap( op::ule ), a, b ); }
        static pv op_sgt( pr a, pr b ) { return bin( wrap( op::sgt ), a, b ); }
        static pv op_sge( pr a, pr b ) { return bin( wrap( op::sge ), a, b ); }
        static pv op_slt( pr a, pr b ) { return bin( wrap( op::slt ), a, b ); }
        static pv op_sle( pr a, pr b ) { return bin( wrap( op::sle ), a, b ); }

        static pv op_foeq( pr a, pr b ) { return bin( wrap( op::foeq ), a, b ); }
        static pv op_fogt( pr a, pr b ) { return bin( wrap( op::fogt ), a, b ); }
        static pv op_foge( pr a, pr b ) { return bin( wrap( op::foge ), a, b ); }
        static pv op_folt( pr a, pr b ) { return bin( wrap( op::folt ), a, b ); }
        static pv op_fole( pr a, pr b ) { return bin( wrap( op::fole ), a, b ); }
        static pv op_fone( pr a, pr b ) { return bin( wrap( op::fone ), a, b ); }
        static pv op_ford( pr a, pr b ) { return bin( wrap( op::ford ), a, b ); }
        static pv op_funo( pr a, pr b ) { return bin( wrap( op::funo ), a, b ); }
        static pv op_fueq( pr a, pr b ) { return bin( wrap( op::fueq ), a, b ); }
        static pv op_fugt( pr a, pr b ) { return bin( wrap( op::fugt ), a, b ); }
        static pv op_fuge( pr a, pr b ) { return bin( wrap( op::fuge ), a, b ); }
        static pv op_fult( pr a, pr b ) { return bin( wrap( op::fult ), a, b ); }
        static pv op_fule( pr a, pr b ) { return bin( wrap( op::fule ), a, b ); }
        static pv op_fune( pr a, pr b ) { return bin( wrap( op::fune ), a, b ); }

        static pv op_ffalse( pr a, pr b ) { return bin( wrap( op::ffalse ), a, b ); }
        static pv op_ftrue ( pr a, pr b ) { return bin( wrap( op::ftrue  ), a, b ); }

        static pv op_trunc  ( pr a, bw b ) { return un( wrap( op::trunc  , b ), a ); }
        static pv op_fptrunc( pr a, bw b ) { return un( wrap( op::fptrunc, b ), a ); }
        static pv op_sitofp ( pr a, bw b ) { return un( wrap( op::sitofp , b ), a ); }
        static pv op_uitofp ( pr a, bw b ) { return un( wrap( op::uitofp , b ), a ); }
        static pv op_zext   ( pr a, bw b ) { return un( wrap( op::zext   , b ), a ); }
        static pv op_zfit   ( pr a, bw b ) { return un( wrap( op::zfit   , b ), a ); }
        static pv op_sext   ( pr a, bw b ) { return un( wrap( op::sext   , b ), a ); }
        static pv op_fpext  ( pr a, bw b ) { return un( wrap( op::fpext  , b ), a ); }
        static pv op_fptosi ( pr a, bw b ) { return un( wrap( op::fptosi , b ), a ); }
        static pv op_fptoui ( pr a, bw b ) { return un( wrap( op::fptoui , b ), a ); }

        static pv op_concat ( pr a, pr b ) { return bin( wrap( op::concat ), a, b ); }
        static pv op_extract( pr, bw, bw ) { return mixin::fail(); }

        template< typename scal >
        static void op_store( pr a, const scal &s, bw w )
        {
            auto op = wrap( op::store, w );
            op( a->first, s );
            op( a->second, s );
        }

        static pv op_load( pr a, bw w )
        {
            return un( wrap( op::load, w ), a );
        }

        static void assume( product &a, bool c )
        {
            auto op = op::wrapr( op::assume, c );
            op( a->first );
            op( a->second );
        }

        static void dump( pr /* a */ )
        {
            __builtin_unreachable();
        }

        static std::string trace( pr /* a */ )
        {
            __builtin_unreachable();
        }

        template< typename stream >
        friend stream& operator<<( stream &os, pr a )
        {
            return os << "(" << a->first << ", " << a->second << ")";
        }

        // backward operations
        static void bop_add ( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::add ), r, a, b ); }
        static void bop_sub ( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::sub ), r, a, b ); }
        static void bop_mul ( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::mul ), r, a, b ); }
        static void bop_sdiv( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::sdiv ), r, a, b ); }
        static void bop_udiv( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::udiv ), r, a, b ); }
        static void bop_srem( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::srem ), r, a, b ); }
        static void bop_urem( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::urem ), r, a, b ); }

        static void bop_fadd( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::fadd ), r, a, b ); }
        static void bop_fsub( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::fsub ), r, a, b ); }
        static void bop_fmul( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::fmul ), r, a, b ); }
        static void bop_fdiv( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::fdiv ), r, a, b ); }
        static void bop_frem( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::frem ), r, a, b ); }

        static void bop_shl ( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::shl ), r, a, b ); }
        static void bop_ashr( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::ashr ), r, a, b ); }
        static void bop_lshr( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::lshr ), r, a, b ); }
        static void bop_and ( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::op_and ), r, a, b ); }
        static void bop_or  ( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::op_or ), r, a, b ); }
        static void bop_xor ( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::op_xor ), r, a, b ); }

        static void bop_eq ( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::eq ), r, a, b ); }
        static void bop_ne ( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::ne ), r, a, b ); }
        static void bop_ugt( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::ugt ), r, a, b ); }
        static void bop_uge( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::uge ), r, a, b ); }
        static void bop_ult( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::ult ), r, a, b ); }
        static void bop_ule( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::ule ), r, a, b ); }
        static void bop_sgt( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::sgt ), r, a, b ); }
        static void bop_sge( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::sge ), r, a, b ); }
        static void bop_slt( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::slt ), r, a, b ); }
        static void bop_sle( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::sle ), r, a, b ); }

        static void bop_foeq( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::foeq ), r, a, b ); }
        static void bop_fogt( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::fogt ), r, a, b ); }
        static void bop_foge( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::foge ), r, a, b ); }
        static void bop_folt( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::folt ), r, a, b ); }
        static void bop_fole( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::fole ), r, a, b ); }
        static void bop_ford( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::ford ), r, a, b ); }
        static void bop_funo( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::funo ), r, a, b ); }
        static void bop_fueq( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::fueq ), r, a, b ); }
        static void bop_fugt( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::fugt ), r, a, b ); }
        static void bop_fuge( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::fuge ), r, a, b ); }
        static void bop_fult( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::fult ), r, a, b ); }
        static void bop_fule( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::fule ), r, a, b ); }

        static void bop_ffalse( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::ffalse ), r, a, b ); }
        static void bop_ftrue ( pr r, pr a, pr b ) { bin_bop( wrap( op::backward::ftrue ), r, a, b ); }

        static void bop_trunc  ( pr r, pr a ) { un_bop( wrap( op::backward::trunc ), r, a ); }
        static void bop_fptrunc( pr r, pr a ) { un_bop( wrap( op::backward::fptrunc ), r, a ); }
        static void bop_sitofp ( pr r, pr a ) { un_bop( wrap( op::backward::sitofp ), r, a ); }
        static void bop_uitofp ( pr r, pr a ) { un_bop( wrap( op::backward::uitofp ), r, a ); }
        static void bop_zext   ( pr r, pr a ) { un_bop( wrap( op::backward::zext ), r, a ); }
        static void bop_zfit   ( pr r, pr a ) { un_bop( wrap( op::backward::zfit ), r, a ); }
        static void bop_sext   ( pr r, pr a ) { un_bop( wrap( op::backward::sext ), r, a ); }
        static void bop_fpext  ( pr r, pr a ) { un_bop( wrap( op::backward::fpext ), r, a ); }
        static void bop_fptosi ( pr r, pr a ) { un_bop( wrap( op::backward::fptosi ), r, a ); }
        static void bop_fptoui ( pr r, pr a ) { un_bop( wrap( op::backward::fptoui ), r, a ); }
    };
}
