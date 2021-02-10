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

#include <cstdint>

#include <lava/support/array.hpp>

namespace __lava
{
    using bitwidth_t = uint8_t;
    using tag_t = uint8_t;

    constexpr tag_t invalid_tag = std::numeric_limits< tag_t >::max();

    struct tagged_array : array< uint8_t >
    {
        using base = array;
        using typename base::iterator;

        template< typename F >
        void move_tag( F f )
        {
            uint8_t tag = base::back();
            f();
            base::back() = tag;
        }

        using base::base;

        tagged_array() { base::resize( 1, invalid_tag ); }
        tagged_array( const tagged_array &o, construct_shared_t s ) : base( o, s ) {}

        template< typename derived >
        derived storage_copy() const
        {
            tagged_array storage{ base::size(), base::begin(), base::end() };
            return { storage.disown(), derived::construct_shared };
        }

        auto  end()       { return base::end() - 1; }
        auto  end() const { return base::end() - 1; }
        auto cend() const { return base::end() - 1; }

        auto  rbegin()       { return base::rbegin() + 1; }
        auto  rbegin() const { return base::rbegin() + 1; }
        auto crbegin() const { return base::rbegin() + 1; }

        auto       &back()       { return *( end() - 1 ); }
        const auto &back() const { return *( end() - 1 ); }
        uint8_t    &tag()        { return base::back(); }
        uint8_t     tag()  const { return base::back(); }

        unsigned size()  const { return base::size() - 1; }
        bool    empty() const { return size() == 0; }
        void    _grow() { base::push_back( 0 ); }

        base       &raw()       { return *this; }
        const base &raw() const { return *this; }

        void *unsafe_ptr() const
        {
            auto ptr = static_cast< const void * >( &*base::begin() );
            return const_cast< void * >( ptr );
        }

        void push_back( uint8_t v )          { move_tag( [&]{ base::back() = v; _grow(); } ); }
        void emplace_back( uint8_t v )       { move_tag( [&]{ base::back() = v; _grow(); } ); }
        void resize( unsigned sz )           { move_tag( [&]{ base::resize( sz + 1 ); } ); }
        void clear()                         { move_tag( [&]{ base::resize( 1 ); } ); }
        void pop_back()                      { move_tag( [&]{ base::pop_back(); } ); }
        void insert( iterator i, uint8_t v ) { move_tag( [&]{ base::insert( i, v ); } ); }
    };

    template< typename type >
    struct tagged_storage : tagged_array
    {
        using base = tagged_array;
        using stored_t = type;

        tagged_storage( void *v, construct_shared_t s ) : base( v, s ) {}

        template< typename... args_t >
        tagged_storage( args_t && ... args )
        {
            resize( sizeof( type ) );
            new ( &**this ) type( std::forward< args_t >( args ) ... );
        }

        tagged_storage( const tagged_storage &o ) : tagged_storage( o.get() ) {}
        tagged_storage( tagged_storage &&o ) : tagged_storage( std::move( o.get() ) ) {}
        tagged_storage &operator=( const tagged_storage &o ) { get() = o.get(); return *this; }

        ~tagged_storage()
        {
            if ( begin() )
                get().~type();
        }

        const type &operator*() const { return *reinterpret_cast< const type * >( begin() ); }
        type       &operator*()       { return *reinterpret_cast< type * >( begin() ); }

        const type *operator->() const { return &**this; }
        type       *operator->()       { return &**this; }

        type       &get()       { return **this; }
        const type &get() const { return **this; }
    };

    struct base {};

} // namespace __lava
