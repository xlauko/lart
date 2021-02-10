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

    template< typename base >
    struct domain_ref : base
    {
        domain_ref( const base &v ) : base( v.unsafe_ptr(), construct_shared ) {}
        explicit domain_ref( void *v ) : base( v, construct_shared ) {}

        ~domain_ref() { this->disown(); }

        base copy_from() const { return this->template storage_copy< base >(); }
    };

    using i1  = bool;
    using i8  = uint8_t;
    using i16 = uint16_t;
    using i32 = uint32_t;
    using i64 = uint64_t;

    using si8  = int8_t;
    using si16 = int16_t;
    using si32 = int32_t;
    using si64 = int64_t;

    using f32 = float;
    using f64 = double;
    struct array_ref { void *base; size_t size; };

    template< typename type >
    constexpr int bitwidth_v = std::is_same_v< type, bool > ? 1 : sizeof( type ) * 8;

    template< typename T > struct number_type { using type = T; };
    template< bool, int > struct number_;

    template<> struct number_< true, 1 >  : number_type< bool > {};
    template<> struct number_< true, 8 >  : number_type< int8_t > {};
    template<> struct number_< true, 16 > : number_type< int16_t > {};
    template<> struct number_< true, 32 > : number_type< int32_t > {};
    template<> struct number_< true, 64 > : number_type< int64_t > {};

    template<> struct number_< false, 1 >  : number_type< bool > {};
    template<> struct number_< false, 8 >  : number_type< uint8_t > {};
    template<> struct number_< false, 16 > : number_type< uint16_t > {};
    template<> struct number_< false, 32 > : number_type< uint32_t > {};
    template<> struct number_< false, 64 > : number_type< uint64_t > {};

    template< bool s, int w >
    using number = typename number_< s, w >::type;

    template< bool, bool, int > struct value_;

    template< bool s, int w >
    struct value_< false /* non-pointer */, s, w > : number_< s, w > {};

    template< bool s, int w >
    struct value_< true /* pointer */, s, w >
    {
        using type = std::add_pointer_t< number< s, w > >;
    };

    template< bool p, bool s, int w >
    using value = typename value_< p, s, w >::type;

    template< bool s, int w, typename t >
    static value< std::is_pointer_v< t >, s, w > cast( t v )
    {
        return static_cast< decltype( cast< s, w >( v ) ) >( v );
    }

    template< bool signedness, typename fn_t, typename... args_t >
    auto call( bitwidth_t bw, fn_t fn, args_t&&... args )
    {
        switch ( bw ) {
            case 1 : return fn( cast< signedness,  1 >( args )...  );
            case 8 : return fn( cast< signedness,  8 >( args )...  );
            case 16: return fn( cast< signedness, 16 >( args )...  );
            case 32: return fn( cast< signedness, 32 >( args )...  );
            case 64: return fn( cast< signedness, 64 >( args )...  );
        }
        __builtin_unreachable();
    }

    static constexpr auto callu = []( const auto & ... xs ) { return call< false >( xs... ); };
    static constexpr auto calls = []( const auto & ... xs ) { return call< true  >( xs... ); };    struct array_ref { void *base; size_t size; };

    template< typename type >
    constexpr int bitwidth_v = std::is_same_v< type, bool > ? 1 : sizeof( type ) * 8;

    template< typename T > struct number_type { using type = T; };
    template< bool, int > struct number_;

    template<> struct number_< true, 1 >  : number_type< bool > {};
    template<> struct number_< true, 8 >  : number_type< int8_t > {};
    template<> struct number_< true, 16 > : number_type< int16_t > {};
    template<> struct number_< true, 32 > : number_type< int32_t > {};
    template<> struct number_< true, 64 > : number_type< int64_t > {};

    template<> struct number_< false, 1 >  : number_type< bool > {};
    template<> struct number_< false, 8 >  : number_type< uint8_t > {};
    template<> struct number_< false, 16 > : number_type< uint16_t > {};
    template<> struct number_< false, 32 > : number_type< uint32_t > {};
    template<> struct number_< false, 64 > : number_type< uint64_t > {};

    template< bool s, int w >
    using number = typename number_< s, w >::type;

    template< bool, bool, int > struct value_;

    template< bool s, int w >
    struct value_< false /* non-pointer */, s, w > : number_< s, w > {};

    template< bool s, int w >
    struct value_< true /* pointer */, s, w >
    {
        using type = std::add_pointer_t< number< s, w > >;
    };

    template< bool p, bool s, int w >
    using value = typename value_< p, s, w >::type;

    template< bool s, int w, typename t >
    static value< std::is_pointer_v< t >, s, w > cast( t v )
    {
        return static_cast< decltype( cast< s, w >( v ) ) >( v );
    }

    template< bool signedness, typename fn_t, typename... args_t >
    auto call( bitwidth_t bw, fn_t fn, args_t&&... args )
    {
        switch ( bw ) {
            case 1 : return fn( cast< signedness,  1 >( args )...  );
            case 8 : return fn( cast< signedness,  8 >( args )...  );
            case 16: return fn( cast< signedness, 16 >( args )...  );
            case 32: return fn( cast< signedness, 32 >( args )...  );
            case 64: return fn( cast< signedness, 64 >( args )...  );
        }
        __builtin_unreachable();
    }

    static constexpr auto callu = []( const auto & ... xs ) { return call< false >( xs... ); };
    static constexpr auto calls = []( const auto & ... xs ) { return call< true  >( xs... ); };

    struct base {};

} // namespace __lava
