/*
 * (c) 2016 Jan Mrázek <email@honzamrazek.cz>
 * (c) 2019 Petr Ročkai <code@fixp.eu>
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

#include <runtime/vector.hpp>

#include <algorithm>
#include <functional>

namespace __lart::rt
{
namespace {

    struct less_map
    {
        template< typename T >
        auto operator()( const T &a, const T &b ) const -> decltype( a.first < b.first )
        {
            return a.first < b.first;
        }

        template< typename T, typename K >
        auto operator()( const T &a, const K &b ) const -> decltype( a.first < b )
        {
            return a.first < b;
        }

        template< typename T, typename K >
        auto operator()( const K &a, const T &b ) const -> decltype( a < b.first )
        {
            return a < b.first;
        }
    };

    struct derive_eq  {};
    struct derive_ord {};

    template< typename T >
    constexpr bool is_eq_v = std::is_base_of_v< derive_eq, T >;

    template< typename T >
    constexpr bool is_ord_v = std::is_base_of_v< derive_ord, T >;

    template< typename T > using eq_t  = std::enable_if_t< is_eq_v< T >, bool >;
    template< typename T > using ord_t = std::enable_if_t< is_ord_v< T >, bool >;

    template< typename T > eq_t< T > operator!=( const T &a, const T &b ) { return not ( a == b ); }

    template< typename T > std::enable_if_t< !is_eq_v< T > && is_ord_v< T >, bool >
    operator!=( const T &a, const T &b ) { return a < b || b < a; }

    template< typename T > std::enable_if_t< !is_eq_v< T > && is_ord_v< T >, bool >
    operator==( const T &a, const T &b ) { return not ( a != b ); }

    template< typename T > ord_t< T > operator> ( const T &a, const T &b ) { return b < a; }
    template< typename T > ord_t< T > operator>=( const T &a, const T &b ) { return not ( a < b ); }
    template< typename T > ord_t< T > operator<=( const T &a, const T &b ) { return not ( b < a ); }

    template< typename T >
    auto operator<( const T &a, const T &b ) -> decltype( a.as_tuple() < b.as_tuple() )
    {
        return a.as_tuple() < b.as_tuple();
    }

    template< typename cmp_ = std::less<> >
    struct std_sort
    {
        using cmp = cmp_;

        template< typename it >
        static void sort( it begin, it end )
        {
            std::sort( begin, end, cmp() );
        }

        template< typename It, typename K >
        static It lower_bound( It begin, It end, const K &v )
        {
            return std::lower_bound( begin, end, v, cmp() );
        }
    };

    template< typename cmp_ = std::less<> >
    struct insert_sort
    {
        using cmp = cmp_;

        template< typename It, typename K >
        static It lower_bound( It begin, It end, const K &v )
        {
            if ( begin == end )
                return begin;
            auto pivot = begin + ( end - begin ) / 2;
            if ( cmp()( *pivot, v ) )
                return lower_bound( pivot + 1, end, v );
            else
                return lower_bound( begin, pivot, v );
        }

        template< typename It >
        static void sort( It begin, It end )
        {
            int i = 1;
            while ( i < end - begin )
            {
                int j = i;
                while ( j > 0 && cmp()( begin[ j ], begin[ j - 1 ] ) )
                {
                    std::swap( begin[ j ], begin[ j - 1 ] );
                    -- j;
                }
                ++ i;
            }
        }
    };

    template< typename key_t, typename sort = std_sort<>, typename container = std::vector< key_t > >
    struct array_set : derive_eq, derive_ord
    {
        using cmp = typename sort::cmp;
        using key_type = key_t;
        using value_type = key_t;

        using size_type = typename container::size_type;
        using iterator = typename container::iterator;
        using const_iterator = typename container::const_iterator;
        using reference = value_type&;
        using const_reference = const value_type&;

        container _container;

        auto begin() { return _container.begin(); }
        auto begin() const { return _container.cbegin(); }
        auto cbegin() const { return _container.cbegin(); }
        auto end() { return _container.end(); }
        auto end() const { return _container.cend(); }
        auto cend() const { return _container.cend(); }

        auto empty() const { return _container.empty(); }
        auto size() const { return _container.size(); }

        void clear() noexcept { _container.clear(); }

        array_set() = default;
        array_set( std::initializer_list< value_type > il ) : _container( il )
        {
            sort::sort( begin(), end() );
        }

        std::pair< iterator, bool > insert( const_reference value )
        {
            auto it = find( value.first );
            if ( it != cend() )
                return { it, false };
            _container.push_back( value );
            sort::sort( begin(), end() );
            return { find( value.first ), true };
        }

        template< class... Args >
        std::pair< iterator, bool > emplace( Args&&... args )
        {
            _container.emplace_back( std::forward< Args >( args )... );
            auto it = sort::lower_bound( begin(), end() - 1, _container.back() );
            if ( it == end() - 1 || !eq( *it, _container.back() ) )
            {
                // TODO fix copying of value
                auto &item = _container.back();
                // TODO fix swapping
                sort::sort( begin(), end() );
                return { find( item ), true };
            }

            // TODO fix copying of value
            auto &item = *it;
            _container.pop_back();
            return { find( item ), false };
        }

        void erase( iterator pos )
        {
            using std::swap;
            swap( *pos, _container.back() );
            _container.pop_back();
        }

        template< typename K >
        auto erase( const K& key ) -> decltype( cmp()( key, *begin() ), size_type() )
        {
            auto it = find( key );
            if ( it == end() )
                return 0;
            erase( it );
            return 1;
        }

        void swap( array_set &other )
        {
            using std::swap;
            swap( _container, other._container );
        }

        template< typename K >
        static bool eq( const key_t &a, const K &b )
        {
            return !cmp()( a, b ) && !cmp()( b, a );
        }

        template< typename K >
        size_type count( const K &key ) const
        {
            return find( key ) == cend() ? 0 : 1;
        }

        template< typename K >
        iterator find( const K &key )
        {
            auto elem = sort::lower_bound( begin(), end(), key );
            return elem != end() && eq( *elem, key ) ? elem : end();
        }

        template< typename K >
        const_iterator find( const K& key ) const
        {
            auto elem = sort::lower_bound( cbegin(), cend(), key );
            return elem != cend() && eq( *elem, key ) ? elem : cend();
        }

        bool operator<( const array_set &o ) const { return _container < o._container; }
        bool operator==( const array_set &o ) const { return _container == o._container; }
    };

    template< typename key_t, typename val_t, typename sort = std_sort< less_map >,
              typename container = std::vector< std::pair< key_t, val_t > > >
    struct array_map : array_set< std::pair< key_t, val_t >, sort, container >
    {
        using key_type = key_t;
        using mapped_type = val_t;
        using value_type = std::pair< key_t, val_t >;
        using reference = value_type &;
        using const_reference = const value_type &;

        val_t &operator[]( const key_t& key )
        {
            auto it = this->find( key );
            if ( it != this->end() )
                return it->second;
            return this->emplace( key, val_t() ).first->second;
        }

        template< typename K >
        val_t &at( const K& key )
        {
            auto it = this->find( key );
            if ( it == this->end() )
                __builtin_trap();
            return it->second;
        }

        template< typename K >
        const val_t &at( const K& key ) const
        {
            auto it = this->find( key );
            if ( it == this->cend() )
                __builtin_trap();
            return it->second;
        }
    };

}
    template < typename Key, typename Val>
    using array_map = array_map< Key, Val, insert_sort< less_map >,
                                 vector< std::pair< Key, Val > > >;
} // namespace __lart::rt