/*
 * (c) 2016 Jan Mrázek <email@honzamrazek.cz>
 * (c) 2018 Vladimír Štill <xstill@fi.muni.cz>
 * (c) 2019 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <cstddef>
#include <iterator>
#include <memory>
#include <algorithm>
#include <type_traits>

namespace __lart::rt
{
    struct construct_shared_t {};

    static constexpr construct_shared_t construct_shared;

    template< typename T >
    struct array
    {
        using size_type = unsigned;
        using value_type = T;
        using iterator = T *;
        using const_iterator = const T *;
        using reverse_iterator = std::reverse_iterator< iterator >;
        using const_reverse_iterator = std::reverse_iterator< const_iterator >;
        using difference_type = std::ptrdiff_t;
        using reference = T&;
        using const_reference = T const&;

        /* Allow construction of multiple array instances that point to the same
         * memory. Objects constructed in this manner must never be destroyed but
         * must be disowned instead. */

        static constexpr bool nothrow_dtor = std::is_nothrow_destructible_v< T >;
        static constexpr bool nothrow_copy = std::is_nothrow_copy_constructible_v< T >;
        static constexpr bool nothrow_move = std::is_nothrow_move_constructible_v< T >;
        static constexpr bool is_trivial   = std::is_trivial_v< T >;

        struct items
        {
            T *get() noexcept {
                return reinterpret_cast< T* >(reinterpret_cast< size_type* >( this ) + 1);
            }
        };

        array() noexcept = default;
        ~array() noexcept( nothrow_dtor ) { clear(); }

        array( void *ptr, construct_shared_t ) noexcept
            : _data( reinterpret_cast< items* >( reinterpret_cast< size_type* >( ptr ) - 1 ) )
        {}

        array( const array& other ) noexcept ( nothrow_copy )
            : array( other.size(), other.begin(), other.end() )
        { }

        array( array&& other ) noexcept { swap( other ); };

        array( std::initializer_list< T > ilist ) noexcept ( nothrow_copy ) :
            array( ilist.size(), ilist.begin(), ilist.end() )
        { }

        explicit array( size_type size, const T &val = T() ) noexcept( nothrow_copy )
        {
            _resize( size );
            uninit_fill( begin(), end(), val );
        }

        template< typename It >
        array( size_type size, It b, It e ) noexcept ( nothrow_copy )
        {
            append( size, b, e );
        }

        array& operator=( array other ) noexcept ( nothrow_copy )
        {
            swap( other );
            return *this;
        }

        void *disown()
        {
            void *rv = _data;
            _data = nullptr;
            return rv;
        }

        template< typename It >
        void assign( size_type size, It b, It e ) noexcept ( nothrow_copy )
        {
            _clear();
            _resize( size );
            uninit_copy( b, e, begin() );
        }

        void swap( array& other ) noexcept
        {
            using std::swap;
            swap( _data, other._data );
        }

        inline iterator begin() noexcept { return _data ?_data->get() : nullptr; }
        inline const_iterator begin() const noexcept { return _data ?_data->get() : nullptr; }
        inline const_iterator cbegin() const noexcept { return _data ?_data->get() : nullptr; }

        inline iterator end() noexcept { return _data ? _data->get() + size() : nullptr; }
        inline const_iterator end() const noexcept { return _data ? _data->get() + size() : nullptr; }
        inline const_iterator cend() const noexcept { return _data ? _data->get() + size() : nullptr; }

        inline reverse_iterator rbegin() noexcept { return reverse_iterator( end() ); }
        inline const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator( end() ); }
        inline const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator( end() ); }

        inline reverse_iterator rend() noexcept { return reverse_iterator( begin() ); }
        inline const_reverse_iterator rend() const noexcept { return const_reverse_iterator( begin() ); }
        inline const_reverse_iterator crend() const noexcept { return const_reverse_iterator( begin() ); }

        T& back() noexcept { return *( end() - 1 ); }
        const T& back() const noexcept { return *( end() - 1 ); }
        T& front() noexcept { return *begin(); }
        const T& front() const noexcept { return *begin(); }

        [[nodiscard]] inline bool empty() const noexcept { return !_data; }
        [[nodiscard]] inline size_type size() const noexcept
        {
            return empty() ? 0 : *reinterpret_cast< size_type* >( _data );
        }

        inline void clear() noexcept ( nothrow_dtor )
        {
            if ( empty() )
                return;
            _clear();
            free( _data );
            _data = nullptr;
        }

        void push_back( const T& t ) noexcept ( nothrow_copy )
        {
            _resize( size() + 1 );
            new ( &back() ) T( t );
        }

        template< typename It >
        void append( size_type count, It b, It e ) noexcept ( is_trivial )
        {
            size_type oldsz = size();
            _resize( oldsz + count );
            uninit_copy( b, e, begin() + oldsz );
        }

        template < typename... Args >
        T& emplace_back( Args&&... args )
        {
            _resize( size() + 1 );
            new ( &back() ) T( std::forward< Args >( args )... );
            return back();
        }

        void pop_back() noexcept ( nothrow_dtor )
        {
            back().~T();
            _resize( size() - 1 );
        }

        iterator insert( iterator where, const T &val ) noexcept ( nothrow_copy && nothrow_move )
        {
            _resize( size() + 1 );
            for ( iterator i = end() - 1; i > where; --i )
            {
                new ( i ) T( std::move( *( i - 1 ) ) );
                ( i - 1 )->~T();
            }
            new ( where ) T( val );
            return where;
        }


        T& operator[]( size_type idx ) noexcept { return _data->get()[ idx ]; }
        const T& operator[]( size_type idx ) const noexcept { return _data->get()[ idx ]; }

        void uninit_fill( iterator b, iterator e, const T &val ) noexcept ( nothrow_copy )
        {
            if constexpr ( nothrow_copy )
                for ( auto i = b; i != e; ++i )
                    new ( i ) T( val );
            else
                std::uninitialized_fill( b, e, val );
        }

        template< typename from_t >
        void uninit_copy( from_t b, from_t e, iterator to ) noexcept ( nothrow_copy )
        {
            if constexpr ( nothrow_copy )
                for ( auto i = b; i != e; ++i )
                    new ( to++ ) T( *i );
            else
                std::uninitialized_copy( b, e, to );
        }

        void resize( size_type n, const T &val = T() ) noexcept( nothrow_copy )
        {
            size_type old = size();
            _resize( n );

            if ( n > old )
                uninit_fill( begin() + old, end(), val );
        }

        void _resize( size_type n ) noexcept
        {
            if ( n == 0 ) {
                free( _data );
                _data = nullptr;
            }
            else if ( empty() ) {
                _data = static_cast< items* >( malloc( n * sizeof( T ) + sizeof( size_type ) ) );
                (*reinterpret_cast< size_type* >( _data )) = n;
            }
            else
            {
                _data = static_cast< items * >( realloc( _data, n * sizeof( T ) + sizeof( size_type ) ) );
                (*reinterpret_cast< size_type* >( _data )) = n;
            }
        }

        void _clear() noexcept ( nothrow_dtor )
        {
            auto s = size();
            for ( auto i = begin(); i < begin() + s; ++i )
                i->~T();
        }

        bool operator==( const array &o ) const noexcept
        {
            return size() == o.size() && std::equal( begin(), end(), o.begin() );
        }

        bool operator<( const array &o ) const noexcept
        {
            return std::lexicographical_compare( begin(), end(), o.begin(), o.end() );
        }

    private:
        items *_data = nullptr;
    };

} // namespace __lart::rt
