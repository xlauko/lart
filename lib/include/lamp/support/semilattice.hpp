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

#include <lava/constant.hpp>
#include <lava/support/base.hpp>
#include <lava/support/tristate.hpp>
#include <lava/support/operations.hpp>

#include "domain_list.hpp"

namespace __lamp
{
    using bw = __lava::bitwidth_t;
    using tristate = __lava::tristate;
    using constant = __lava::constant;

    template< typename base >
    using domain_ref = __lava::domain_ref< base >;
  
    template< typename type, typename tag >
    struct tagged : type, tag
    {
        using type::type;
        using unwrap = type;
    };

    struct index_tag {};
    struct scalar_tag {};

    template< typename dom_t > using index_t  = tagged< dom_t, index_tag >;
    template< typename dom_t > using scalar_t = tagged< dom_t, scalar_tag >;

     namespace op = __lava::op;

    template< typename sl >
    struct semilattice : __lava::tagged_array, __lava::domain_mixin< semilattice< sl > >
    {
        using base = __lava::tagged_array;
        using doms = typename sl::doms;
        using self = semilattice;
        using sref = const self &;

        template< int idx > using dom_type = typename doms::template type< idx >;
        template< typename type > static constexpr int dom_idx = doms::template idx< type >;

        semilattice( const semilattice& ) = delete;
        semilattice( semilattice&& ) noexcept = default;

        semilattice( void *v, __lart::construct_shared_t s ) : base( v, s ) {}

        template< typename dom_t, typename = std::enable_if_t< doms::template idx< dom_t > >= 0 > >
        semilattice( dom_t &&v ) : base( v.disown(), __lart::construct_shared )
        {
            base::tag() = doms::template idx< dom_t >;
        }

        semilattice& operator=( const semilattice& ) = delete;
        semilattice& operator=( semilattice&& ) noexcept = default;

        struct index_w : domain_ref< self >  { using domain_ref< self >::domain_ref; };
        struct scalar_w : domain_ref< self > { using domain_ref< self >::domain_ref; };

        static constexpr int join( int a ) { return a; }

        template< typename... args_t >
        static constexpr int join( int a, int b, args_t... args )
        {
            if ( a >= 0 && b >= 0 )
                return sl::join( a, join( b, args... ) );
            return join( a >= 0 ? a : b, args... );
        }

        template< typename to, typename from >
        __inline static auto lift_to( const from &f )
        {
            if constexpr ( std::is_base_of_v< index_tag, from > )
            {
                using orig = typename from::unwrap;
                constexpr int goal = join( dom_idx< orig >, dom_idx< typename to::index_dom > );
                static_assert( goal >= 0 );
                return self::lift_to< dom_type< goal >, orig >( f );
            }
            else if constexpr ( std::is_base_of_v< scalar_tag, from > )
            {
                using orig = typename from::unwrap;
                constexpr int orig_idx = dom_idx< orig >;
                ASSERT_EQ( f.tag(), orig_idx );
                constexpr int goal = join( orig_idx, dom_idx< typename to::scalar_dom > );
                static_assert( goal >= 0 );
                return self::lift_to< dom_type< goal >, orig >( f );
            }
            else if constexpr ( std::is_same_v< from, to > )
                return f;
            else if constexpr ( std::is_trivial_v< from > )
                return f;
            else
                return to::lift( f );
        }

        template< typename op_t, int idx = 0, typename... args_t >
        __inline static self in_domain( int dom, op_t op, const args_t & ... args )
        {
            if constexpr ( idx < doms::size )
            {
                constexpr int joined = join( idx, doms::template idx< args_t > ... );
                static_assert( joined >= 0 );
                if ( idx == dom )
                    if constexpr ( joined == idx )
                        return op( lift_to< typename doms::template type< idx > >( args ) ... );

                return in_domain< op_t, idx + 1 >( dom, op, args... );
            }
            else __builtin_unreachable();
        }

        template< typename op_t, typename sl_t, int idx = 0 >
        __inline static auto cast_one( op_t op, const sl_t &v )
        {
            if constexpr ( idx < doms::size )
            {
                constexpr bool is_idx = std::is_same_v< sl_t, index_w >;
                constexpr bool is_scl = std::is_same_v< sl_t, scalar_w >;

                using to_type = typename doms::template type< idx >;
                using index_type = index_t< to_type >;
                using scalar_type = scalar_t< to_type >;
                using coerce1_t = std::conditional_t< is_idx, index_type, to_type >;
                using coerce_t = std::conditional_t< is_scl, scalar_type, coerce1_t >;

                if ( v.tag() == idx )
                {
                    coerce_t coerce( v.unsafe_ptr(), __lart::construct_shared );
                    if constexpr ( std::is_void_v< decltype( op( coerce ) ) > )
                    {
                        op( coerce );
                        coerce.disown();
                        return;
                    }
                    else
                    {
                        auto rv = op( coerce );
                        coerce.disown();
                        return rv;
                    }
                }
                else
                    return self::cast_one< op_t, sl_t, idx + 1 >( op, v );
            }
            return self::cast_one< op_t, sl_t, 0 >( ( __builtin_trap(), op ), v );
        }

        template< typename op_t >
        __inline static auto cast( op_t op ) { return op(); }

        template< typename op_t, typename arg_t, typename... args_t >
        __inline static auto cast( op_t op, const arg_t &a, const args_t & ... args )
        {
            auto rec = [&]( const auto &c ) __inline
            {
                return cast( [&]( const auto & ... cs ) __inline { return op( c, cs... ); }, args... );
            };

            return cast_one( rec, a );
        }

        template< typename op_t, typename... args_t >
        __inline static self op( op_t operation, const args_t & ... args )
        {
            int dom = join( args.tag() ... );

            auto downcasted = [&]( const auto & ... args ) __inline
            {
                return in_domain( dom, operation, args... );
            };

            return cast( downcasted, args... );
        }

        template< typename... Ts >
        static constexpr auto wrap( Ts&&... xs )
            -> decltype( __lava::op::wrap( std::forward< Ts >( xs )... ) )
        {
            return __lava::op::wrap( std::forward< Ts >( xs )... );
        }

        template< typename val_t >
        static self lift( const val_t &val ) { return sl::scalar_lift_dom::lift( val ); }
        static self lift( __lava::array_ref arr ) { return sl::array_lift_dom::lift( arr ); }

        static constant lower( sref a ) { return cast( wrap( op::lower ), a ); }

        template< typename val_t >
        static self any() { return sl::scalar_any_dom::template any< val_t >(); }

        static void assume( sref a, bool c ) { cast( wrap( op::assume, c ), a  ); }
        static tristate to_tristate( sref a )
        {
            return cast( [&]( const auto &v ) { return std::decay_t< decltype( v ) >::to_tristate( v ); }, a );
        }

        static self op_join( sref a, sref b ) { return op( wrap( op::join ), a, b ); }
        static self op_meet( sref a, sref b ) { return op( wrap( op::meet ), a, b ); }

        static self op_add( sref a, sref b ) { return op( wrap( op::add ), a, b ); }
        static self op_sub( sref a, sref b ) { return op( wrap( op::sub ), a, b ); }
        static self op_mul( sref a, sref b ) { return op( wrap( op::mul ), a, b ); }

        static self op_sdiv( sref a, sref b ) { return op( wrap( op::sdiv ), a, b ); }
        static self op_udiv( sref a, sref b ) { return op( wrap( op::udiv ), a, b ); }
        static self op_srem( sref a, sref b ) { return op( wrap( op::srem ), a, b ); }
        static self op_urem( sref a, sref b ) { return op( wrap( op::urem ), a, b ); }

        static self op_and( sref a, sref b ) { return op( wrap( op::_and ), a, b ); }
        static self op_or ( sref a, sref b ) { return op( wrap( op::_or  ), a, b ); }
        static self op_xor( sref a, sref b ) { return op( wrap( op::_xor ), a, b ); }

        static self op_zfit ( sref a, bw b ) { return op( wrap( op::zfit,  b ), a ); }
        static self op_zext ( sref a, bw b ) { return op( wrap( op::zext,  b ), a ); }
        static self op_sext ( sref a, bw b ) { return op( wrap( op::sext,  b ), a ); }
        static self op_trunc( sref a, bw b ) { return op( wrap( op::trunc, b ), a ); }

        static self op_eq ( sref a, sref b ) { return op( wrap( op::eq ), a, b ); }
        static self op_ne ( sref a, sref b ) { return op( wrap( op::ne ), a, b ); }

        static self op_slt( sref a, sref b ) { return op( wrap( op::slt ), a, b ); }
        static self op_sgt( sref a, sref b ) { return op( wrap( op::sgt ), a, b ); }
        static self op_sle( sref a, sref b ) { return op( wrap( op::sle ), a, b ); }
        static self op_sge( sref a, sref b ) { return op( wrap( op::sge ), a, b ); }

        static self op_ult( sref a, sref b ) { return op( wrap( op::ult ), a, b ); }
        static self op_ugt( sref a, sref b ) { return op( wrap( op::ugt ), a, b ); }
        static self op_ule( sref a, sref b ) { return op( wrap( op::ule ), a, b ); }
        static self op_uge( sref a, sref b ) { return op( wrap( op::uge ), a, b ); }

        static self op_shl ( sref a, sref b ) { return op( wrap( op::shl  ), a, b ); }
        static self op_lshr( sref a, sref b ) { return op( wrap( op::lshr ), a, b ); }
        static self op_ashr( sref a, sref b ) { return op( wrap( op::ashr ), a, b ); }

        static self op_foeq( sref a, sref b ) { return op( wrap( op::foeq ), a, b ); }
        static self op_fogt( sref a, sref b ) { return op( wrap( op::fogt ), a, b ); }
        static self op_foge( sref a, sref b ) { return op( wrap( op::foge ), a, b ); }
        static self op_folt( sref a, sref b ) { return op( wrap( op::folt ), a, b ); }
        static self op_fole( sref a, sref b ) { return op( wrap( op::fole ), a, b ); }
        static self op_fone( sref a, sref b ) { return op( wrap( op::fone ), a, b ); }
        static self op_ford( sref a, sref b ) { return op( wrap( op::ford ), a, b ); }
        static self op_funo( sref a, sref b ) { return op( wrap( op::funo ), a, b ); }
        static self op_fueq( sref a, sref b ) { return op( wrap( op::fueq ), a, b ); }
        static self op_fugt( sref a, sref b ) { return op( wrap( op::fugt ), a, b ); }
        static self op_fuge( sref a, sref b ) { return op( wrap( op::fuge ), a, b ); }
        static self op_fult( sref a, sref b ) { return op( wrap( op::fult ), a, b ); }
        static self op_fule( sref a, sref b ) { return op( wrap( op::fule ), a, b ); }
        static self op_fune( sref a, sref b ) { return op( wrap( op::fune ), a, b ); }

        static self op_ffalse( sref a, sref b ) { return op( wrap( op::ffalse ), a, b ); }
        static self op_ftrue ( sref a, sref b ) { return op( wrap( op::ftrue ),  a, b ); }

        static self op_concat ( sref a, sref b ) { return op( wrap( op::concat ), a, b ); }
        static self op_extract( sref a, bw f, bw t ) { return op( wrap( op::extract, f, t ), a ); }
    };

} // namespace __lamp