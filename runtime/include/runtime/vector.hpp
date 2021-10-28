#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <algorithm>
#include <type_traits>

namespace __lart::rt
{
    template< typename T >
    struct vector
    {
        static_assert( std::is_nothrow_destructible_v< T > );

        using size_type = size_t;
        using value_type = T;
        using iterator = T *;
        using const_iterator = const T *;
        using reverse_iterator = std::reverse_iterator< iterator >;
        using const_reverse_iterator = std::reverse_iterator< const_iterator >;
        using difference_type = std::ptrdiff_t;
        using reference = T&;
        using const_reference = T const&;

        ~vector() noexcept { clear(); }

        [[nodiscard]] inline bool empty() const noexcept { return !_size; }
        [[nodiscard]] inline size_type size() const noexcept { return _size; }

        inline iterator begin() noexcept { return _data; }
        inline const_iterator begin() const noexcept { return _data; }
        inline const_iterator cbegin() const noexcept { return _data; }

        inline iterator end() noexcept { return _data + size(); }
        inline const_iterator end() const noexcept { return _data + size(); }
        inline const_iterator cend() const noexcept { return _data + size(); }

        inline reverse_iterator rbegin() noexcept { return reverse_iterator( end() ); }
        inline const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator( end() ); }
        inline const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator( end() ); }

        inline reverse_iterator rend() noexcept { return reverse_iterator( begin() ); }
        inline const_reverse_iterator rend() const noexcept { return const_reverse_iterator( begin() ); }
        inline const_reverse_iterator crend() const noexcept { return const_reverse_iterator( begin() ); }
      
        T& back() noexcept { return *std::prev( end() ); }
        const T& back() const noexcept { return *std::prev( end() ); }
        T& front() noexcept { return *begin(); }
        const T& front() const noexcept { return *begin(); }

        void push_back( const T& t ) noexcept
        {
            _resize( size() + 1 );
            new ( &back() ) T( t );
        }

        template< typename... args_t >
        T& emplace_back( args_t&&... args )
        {
            _resize( size() + 1 );
            new ( &back() ) T( std::forward< args_t >( args )... );
            return back();
        }

        void pop_back() noexcept
        {
            back().~T();
            _resize( size() - 1 );
        }

        T& operator[]( size_type idx ) noexcept { return _data[ idx ]; }
        const T& operator[]( size_type idx ) const noexcept { return _data[ idx ]; }

        inline void clear() noexcept
        {
            if ( empty() )
                return;
            std::destroy( begin(), end() );
            std::free( _data );
            _data = nullptr;
            _size = 0;
        }

        inline void resize( size_type n ) noexcept { _resize( n ); }

    private:

        void _resize( size_type n ) noexcept
        {
            if ( n == 0 ) {
                std::free( _data );
                _data = nullptr;
            } else if ( empty() ) {
                _data = static_cast< T* >( std::malloc( n * sizeof( T ) ) );
            } else {
                _data = static_cast< T* >( std::realloc( _data, n * sizeof( T ) ) );
            }
            _size = n;
        }

        size_type _size = 0;
        T *_data = nullptr;
    };

} // namespace __lart::rt
