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
#include <lava/support/tristate.hpp>
#include <lava/support/base.hpp>

#include <type_traits>
#include <optional>
#include <memory>

namespace __lamp
{
    struct nil
    {
        using co = nil;
        using unique_t = nil;
        static constexpr bool empty = true;
        static constexpr size_t size = 0;

        template< int idx > using type_at = void;

        template< typename T, int = 0 >
        static constexpr int index_of = -1;

        template< template< typename > class f > using map_t = nil;
        template< typename L >                   using cat_t = L;

        template< typename F >     void each( const F & ) const {}
        template< typename F >     nil map( F ) const { return {}; }
        template< typename... Fs > auto match( Fs... ) { return std::nullopt; }
        template< typename L >     auto cat( const L &l ) const { return l; }
        template< typename T >     static constexpr bool has = false;

        template< template< typename > class P >
        [[nodiscard]] constexpr nil filter() const { return {}; }

        template< typename F, typename... args_t >
        auto apply( const F &f, args_t && ... args ) const
        {
            return f( args... );
        }
    };

    template< typename, typename > struct cons;
    template< typename, typename > struct ns;

    template< typename A, typename B >
    auto make_cons( const A &a, const B &b ) { return cons< A, B >( a, b ); }

    template< typename A, typename B >
    struct cons : B
    {
        using car_t = A;
        using cdr_t = B;
        using co = ns< car_t, typename cdr_t::co >;

        car_t _car;

        explicit operator       car_t&()       { return _car; }
        explicit operator const car_t&() const { return _car; }

        car_t       &car()       { return _car; }
        const car_t &car() const { return _car; }
        cdr_t       &cdr()       { return *this; }
        const cdr_t &cdr() const { return *this; }

        template< typename F > void each( const F &f )       { f( car() ); cdr().each( f ); }
        template< typename F > void each( const F &f ) const { f( car() ); cdr().each( f ); }
        template< typename L > auto cat( const L &l ) const { return make_cons( car(), cdr().cat( l ) ); }
        constexpr auto reverse() const { nil rev; return cons_reverse( *this, rev ); }

        template< typename T > auto &get()
        {
            if constexpr ( std::is_same_v< std::remove_reference_t< T >, car_t > )
                return car();
            else
                return cdr().template get< T >();
        }

        template< typename L >
        auto view()
        {
            if constexpr ( std::is_same_v< L, nil > )
                return nil();
            else
            {
                static_assert( has< std::remove_reference_t< typename L::car_t > > );
                auto tail = view< typename L::cdr_t >();
                return cons< typename L::car_t, decltype( tail ) >( get< typename L::car_t >(), tail );
            }
        }

        static constexpr bool empty = false;
        static constexpr size_t size = 1 + cdr_t::size;

        template< int idx >
        using type_at = std::conditional_t< idx == 0, car_t,
                                            typename cdr_t::template type_at< idx - 1 > >;

        template< typename T, int i = 0 > static constexpr int index_of =
            std::is_same_v< T, car_t > ? i : cdr_t::template index_of< T, i + 1 >;

        template< typename T > static constexpr bool has =
            std::is_same_v< T, car_t > || cdr_t::template has< T >;

        template< template< typename > class f >
        using map_t = cons< f< car_t >, typename cdr_t::template map_t< f > >;
        using unique_t = std::conditional_t< cdr_t::unique_t::template has< car_t >, cdr_t, cons >;
        template< typename L > using cat_t = cons< car_t, typename cdr_t::template cat_t< L > >;

        template< typename F, typename... args_t >
        auto apply( const F &f, args_t &&... args ) const
        {
            return cdr().apply( f, car(), std::forward< args_t >( args ) ... );
        }

        template< typename F, typename... args_t >
        auto apply( const F &f, args_t &&... args )
        {
            return cdr().apply( f, car(), std::forward< args_t >( args ) ... );
        }

        template< template< typename > class P >
        constexpr auto filter() const
        {
            if constexpr ( P< car_t >::value )
                return make_cons( car(), cdr().template filter< P >() );
            else
                return cdr().template filter< P >();
        }

        template< typename F >
        auto map( F f )
        {
            auto car_ = f( car() );
            auto cdr_ = cdr().map( f );
            return make_cons( car_, cdr_ );
        }

        cons( const car_t &car, const cdr_t &cdr ) : cdr_t( cdr ), _car( car ) {}
        explicit cons( const car_t &car ) : _car( car ) {}

        template< typename T, typename U, typename... Us >
        cons( const T &t, const U &u, const Us &... us ) : cdr_t( u, us... ), _car( t ) {}

        cons() = default;
    };

    template< typename D >
    constexpr auto cons_reverse( nil, const D &rev )
    {
        return rev;
    }

    template< typename C, typename D >
    constexpr auto cons_reverse( const C &cell, const D &rev )
    {
        return cons_reverse( cell.cdr(), cons< typename C::car_t, D >( cell.car(), rev ) );
    }

    static auto cons_list() { return nil(); }
    static auto cons_list_ref() { return nil(); }

    template< typename T, typename... Ts >
    auto cons_list_ref( T &&t, Ts &&... ts )
    {
        auto tail = cons_list_ref( std::forward< Ts >( ts )... );
        return cons< T, decltype( tail ) >( std::forward< T >( t ), tail );
    }

    template< typename T, typename... Ts >
    auto cons_list( const T &t, const Ts &... ts )
    {
        auto tail = cons_list( ts... );
        return cons< T, decltype( tail ) >( t, tail );
    }
  
    template< typename... Ts > struct cons_list_t__;

    template< typename T, typename... Ts >
    struct cons_list_t__< T, Ts... >
    {
        using type = cons< T, typename cons_list_t__< Ts... >::type >;
    };

    template<> struct cons_list_t__<>
    {
        using type = nil;
    };

    template< typename... Ts >
    using cons_list_t_ = typename cons_list_t__< Ts... >::type;

    template< typename... Ts >
    struct cons_list_t: cons_list_t_< Ts... >
    {
        using cons_list_t_< Ts... >::cons_list_t_;
    };

    template< typename... domains >
    struct domain_list : cons_list_t< domains... >
    {
        template< typename type > static constexpr int idx = domain_list::template index_of< type >;
        template< int idx > using type = typename domain_list::template type_at< idx >;
    };

} // namespace __lamp