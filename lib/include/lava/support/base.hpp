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

#include <cstdint>

#include <runtime/array.hpp>

namespace __lava
{
    using bitwidth_t = uint8_t;
    using tag_t = uint8_t;

    static constexpr tag_t invalid_tag = std::numeric_limits< tag_t >::max();

    struct tagged_array : __lart::array< uint8_t >
    {
        using base = __lart::array< uint8_t >;
        using typename base::iterator;

        using construct_shared_t = __lart::construct_shared_t;

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

        using construct_shared_t = __lart::construct_shared_t;

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
    {ja som asi tiez za VLK
        domain_ref( const base &v ) : base( v.unsafe_ptr(), __lart::construct_shared ) {}
        explicit domain_ref( void *v ) : base( v, __lart::construct_shared ) {}

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
    static constexpr auto calls = []( const auto & ... xs ) { return call< true  >( xs... ); };

    template< typename self_t >
    struct domain_mixin
    {
        using st = self_t;
        using sr = const self_t &;
        using bw = bitwidth_t;

        using index_dom = self_t;
        using scalar_dom = self_t;


        static inline st fail( const char *msg = "" )
        {
            // TODO log messgage
            (void)msg;
            __builtin_trap();
        }

        static st lift_i1( i1 v )   { return st::lift( v ); }
        static st lift_i8( i8 v )   { return st::lift( v ); }
        static st lift_i16( i16 v ) { return st::lift( v ); }
        static st lift_i32( i32 v ) { return st::lift( v ); }
        static st lift_i64( i64 v ) { return st::lift( v ); }

        static st lift_si8( si8 v )   { return st::lift( v ); }
        static st lift_si16( si16 v ) { return st::lift( v ); }
        static st lift_si32( si32 v ) { return st::lift( v ); }
        static st lift_si64( si64 v ) { return st::lift( v ); }

        static st lift_f32( f32 v ) { return st::lift( v ); }
        static st lift_f64( f64 v ) { return st::lift( v ); }

        static st lift_arr( void *v, unsigned s ) { return st::lift( array_ref{ v, s } ); }
        static st lift_ptr( void *v ) { return st::lift( v ); }

        template< typename size >
        static st op_alloca( const size&, bw ) { return fail( "alloca" ); }

        static st op_join( sr, sr ) { return fail( "join" ); }
        static st op_meet( sr, sr ) { return fail( "meet" ); }

        static st op_not ( sr ) { return fail( "not" ); }
        static st op_neg ( sr ) { return fail( "neg" ); }

        static st op_add ( sr, sr ) { return fail( "add" ); }
        static st op_sub ( sr, sr ) { return fail( "sub" ); }
        static st op_mul ( sr, sr ) { return fail( "mul" ); }
        static st op_sdiv( sr, sr ) { return fail( "sdiv" ); }
        static st op_udiv( sr, sr ) { return fail( "udiv" ); }
        static st op_srem( sr, sr ) { return fail( "srem" ); }
        static st op_urem( sr, sr ) { return fail( "urem" ); }

        static st op_fadd( sr, sr ) { return fail( "fadd" ); }
        static st op_fsub( sr, sr ) { return fail( "fsub" ); }
        static st op_fmul( sr, sr ) { return fail( "fmul" ); }
        static st op_fdiv( sr, sr ) { return fail( "fdiv" ); }
        static st op_frem( sr, sr ) { return fail( "frem" ); }

        static st op_shl ( sr, sr ) { return fail( "shl"  ); }
        static st op_ashr( sr, sr ) { return fail( "ashr" ); }
        static st op_lshr( sr, sr ) { return fail( "lshr" ); }
        static st op_and ( sr, sr ) { return fail( "and"  ); }
        static st op_or  ( sr, sr ) { return fail( "or"   ); }
        static st op_xor ( sr, sr ) { return fail( "xor"  ); }

        static st op_eq ( sr, sr ) { return fail( "eq" ); }
        static st op_ne ( sr, sr ) { return fail( "ne" ); }
        static st op_ugt( sr, sr ) { return fail( "ugt" ); }
        static st op_uge( sr, sr ) { return fail( "uge" ); }
        static st op_ult( sr, sr ) { return fail( "ult" ); }
        static st op_ule( sr, sr ) { return fail( "ule" ); }
        static st op_sgt( sr, sr ) { return fail( "sgt" ); }
        static st op_sge( sr, sr ) { return fail( "sge" ); }
        static st op_slt( sr, sr ) { return fail( "slt" ); }
        static st op_sle( sr, sr ) { return fail( "sle" ); }

        static st op_foeq( sr, sr ) { return fail( "foeq" ); }
        static st op_fogt( sr, sr ) { return fail( "fogt" ); }
        static st op_foge( sr, sr ) { return fail( "foge" ); }
        static st op_folt( sr, sr ) { return fail( "folt" ); }
        static st op_fole( sr, sr ) { return fail( "fole" ); }
        static st op_fone( sr, sr ) { return fail( "fone" ); }
        static st op_ford( sr, sr ) { return fail( "ford" ); }
        static st op_funo( sr, sr ) { return fail( "funo" ); }
        static st op_fueq( sr, sr ) { return fail( "fueq" ); }
        static st op_fugt( sr, sr ) { return fail( "fugt" ); }
        static st op_fuge( sr, sr ) { return fail( "fuge" ); }
        static st op_fult( sr, sr ) { return fail( "fult" ); }
        static st op_fule( sr, sr ) { return fail( "fule" ); }
        static st op_fune( sr, sr ) { return fail( "fune" ); }

        static st op_ffalse( sr, sr ) { return fail( "ffalse" ); }
        static st op_ftrue ( sr, sr ) { return fail( "ftrue" ); }

        static st op_trunc  ( sr, bw ) { return fail( "trunc"   ); }
        static st op_fptrunc( sr, bw ) { return fail( "fptrunc" ); }
        static st op_sitofp ( sr, bw ) { return fail( "sitofp"  ); }
        static st op_uitofp ( sr, bw ) { return fail( "uitofp"  ); }
        static st op_zext   ( sr, bw ) { return fail( "zext"    ); }
        static st op_zfit   ( sr, bw ) { return fail( "zfit"    ); }
        static st op_sext   ( sr, bw ) { return fail( "sext"    ); }
        static st op_fpext  ( sr, bw ) { return fail( "fpext"   ); }
        static st op_fptosi ( sr, bw ) { return fail( "fptosi"  ); }
        static st op_fptoui ( sr, bw ) { return fail( "fptoui"  ); }

        static st op_concat ( sr, sr ) { return fail( "concat" ); }
        static st op_extract( sr, bw, bw ) { return fail( "extract" ); }

        template< typename scal > static st op_store( sr, scal, bw ) { return fail( "store" ); }
        template< typename scal > static st op_store_at( sr, sr, scal, bw ) { return fail( "store_at" ); }
        static st op_load( sr, bw ) { return fail( "load" ); }
        static st op_load_at( sr, sr, bw ) { return fail( "load_at" ); }

        static void op_dealloca( sr s ) { s.~self_t(); }
    };

    struct base {};

} // namespace __lava
