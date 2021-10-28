/*
 * (c) 2019 Petr Ročkai <code@fixp.eu>
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

#include <functional>
#include <experimental/iterator>
#include <cassert>

namespace __lava
{
    enum smt_op_type { smt_op_other, smt_op_var, smt_op_const, smt_op_cast, smt_op_resize,
                       smt_op_bool, smt_op_compare };


    struct smt_traits_t
    {
        const char *name;
        unsigned arity:4;
        unsigned imm_bytes:4;
        int bitwidth:10;
        bool floating;
        smt_op_type type:8;

        constexpr bool is_var()    const { return type == smt_op_var; }
        constexpr bool is_const()  const { return type == smt_op_const; }
        constexpr bool is_cast()   const { return type == smt_op_cast; }
        constexpr bool is_resize() const { return type == smt_op_resize; }
        constexpr bool is_integral() const { return !floating; }
        constexpr bool is_float() const { return floating; }
    };

    enum class smt_op : uint8_t
    {
        invalid, eq, neq, join, meet, array,
        var_i1, var_i8, var_i16, var_i32, var_i64, var_f32, var_f64,
        const_i1, const_i8, const_i16, const_i32, const_i64, const_f32, const_f64,
        bv_not, bv_neg,
        bv_sext, bv_zext, bv_trunc, bv_zfit, bv_stofp, bv_utofp,
        fp_ext, fp_trunc, fp_tosbv, fp_toubv,
        bool_and, bool_or, bool_xor, bool_imply, bool_not,
        bv_and, bv_or, bv_xor,
        bv_add, bv_sub, bv_mul, bv_udiv, bv_sdiv, bv_urem, bv_srem,
        bv_shl, bv_lshr, bv_ashr,
        bv_ule, bv_ult, bv_uge, bv_ugt, bv_sle, bv_slt, bv_sge, bv_sgt,
        fp_add, fp_sub, fp_mul, fp_div, fp_rem,
        fp_false, fp_oeq, fp_ogt, fp_oge, fp_olt, fp_ole, fp_one, fp_ord,
        fp_true,  fp_ueq, fp_ugt, fp_uge, fp_ult, fp_ule, fp_une, fp_uno,
        bv_extract, bv_concat,
        load, store
    };

    using smt_varid_t = uint16_t;

    struct [[gnu::packed]] smt_array_type
    {
        smt_varid_t id;

        uint8_t bitwidth;
        enum class type_t : uint8_t { bitvector, floating, array } type;
    };

    static_assert( sizeof( smt_array_type ) == 4 );

    [[gnu::always_inline]] constexpr smt_traits_t smt_traits( smt_op v )
    {
        using op = smt_op;

        switch ( v )
        {
            case op::invalid:    return { "invalid",  0, 0,  0, false, smt_op_other };

            case op::eq:         return { "=",  2, 0, 1, false, smt_op_compare };
            case op::neq:        return { "!=", 2, 0, 1, false, smt_op_compare };

            case op::join:       return { "join", 2, 0, 1, false, smt_op_other };
            case op::meet:       return { "join", 2, 0, 1, false, smt_op_other };

            case op::array:      return { "array", 0, 4,  0, false, smt_op_other };

            case op::var_i1:     return { "var_i1",    0, 2,  1, false, smt_op_var };
            case op::var_i8:     return { "var_i8",    0, 2,  8, false, smt_op_var };
            case op::var_i16:    return { "var_i16",   0, 2, 16, false, smt_op_var };
            case op::var_i32:    return { "var_i32",   0, 2, 32, false, smt_op_var };
            case op::var_i64:    return { "var_i64",   0, 2, 64, false, smt_op_var };
            case op::var_f32:    return { "var_f32",   0, 2, 32,  true, smt_op_var };
            case op::var_f64:    return { "var_f64",   0, 2, 64,  true, smt_op_var };

            case op::const_i1:   return { "const_i1",  0, 1,  1, false, smt_op_const };
            case op::const_i8:   return { "const_i8",  0, 1,  8, false, smt_op_const };
            case op::const_i16:  return { "const_i16", 0, 2, 16, false, smt_op_const };
            case op::const_i32:  return { "const_i32", 0, 4, 32, false, smt_op_const };
            case op::const_i64:  return { "const_i64", 0, 8, 64, false, smt_op_const };
            case op::const_f32:  return { "const_f32", 0, 4, 32,  true, smt_op_const };
            case op::const_f64:  return { "const_f64", 0, 8, 64,  true, smt_op_const };

            case op::bv_not:     return { "bvnot", 1, 0, 1, false, smt_op_other };
            case op::bv_neg:     return { "bvneg", 1, 0, 0, false, smt_op_other };

            case op::bv_sext:    return { "sext",     1, 1, 0, false, smt_op_resize };
            case op::bv_zext:    return { "zext",     1, 1, 0, false, smt_op_resize };
            case op::bv_zfit:    return { "zfit",     1, 1, 0, false, smt_op_resize };
            case op::bv_trunc:   return { "trunc",    1, 1, 0, false, smt_op_resize };
            case op::fp_trunc:   return { "fptrunc",  1, 1, 0,  true, smt_op_resize };
            case op::fp_ext:     return { "fpext",    1, 1, 0,  true, smt_op_resize };

            case op::bv_stofp:   return { "sinttofp", 0, 1, 0, false, smt_op_cast };
            case op::bv_utofp:   return { "uinttofp", 0, 1, 0, false, smt_op_cast };
            case op::fp_tosbv:   return { "fptosint", 0, 1, 0,  true, smt_op_cast };
            case op::fp_toubv:   return { "fptouint", 0, 1, 0,  true, smt_op_cast };

            case op::bool_and:   return { "and",  2, 0, 1, false, smt_op_bool };
            case op::bool_or:    return { "or",   2, 0, 1, false, smt_op_bool };
            case op::bool_xor:   return { "xor",  2, 0, 1, false, smt_op_bool };
            case op::bool_imply: return { "=>",   2, 0, 1, false, smt_op_bool };
            case op::bool_not:   return { "not",  1, 0, 1, false, smt_op_bool };

            case op::bv_and:     return { "bvand",  2, 0, 0, false, smt_op_other };
            case op::bv_or:      return { "bvor",   2, 0, 0, false, smt_op_other };
            case op::bv_xor:     return { "bvxor",  2, 0, 0, false, smt_op_other };
            case op::bv_add:     return { "bvadd",  2, 0, 0, false, smt_op_other };
            case op::bv_sub:     return { "bvsub",  2, 0, 0, false, smt_op_other };
            case op::bv_mul:     return { "bvmul",  2, 0, 0, false, smt_op_other };
            case op::bv_udiv:    return { "bvudiv", 2, 0, 0, false, smt_op_other };
            case op::bv_sdiv:    return { "bvsdiv", 2, 0, 0, false, smt_op_other };
            case op::bv_urem:    return { "bvurem", 2, 0, 0, false, smt_op_other };
            case op::bv_srem:    return { "bvsrem", 2, 0, 0, false, smt_op_other };
            case op::bv_shl:     return { "bvshl",  2, 0, 0, false, smt_op_other };
            case op::bv_lshr:    return { "bvlshr", 2, 0, 0, false, smt_op_other };
            case op::bv_ashr:    return { "bvashr", 2, 0, 0, false, smt_op_other };

            case op::bv_ule:     return { "bvule", 2, 0, 1, false, smt_op_compare };
            case op::bv_ult:     return { "bvult", 2, 0, 1, false, smt_op_compare };
            case op::bv_uge:     return { "bvuge", 2, 0, 1, false, smt_op_compare };
            case op::bv_ugt:     return { "bvugt", 2, 0, 1, false, smt_op_compare };
            case op::bv_sle:     return { "bvsle", 2, 0, 1, false, smt_op_compare };
            case op::bv_slt:     return { "bvslt", 2, 0, 1, false, smt_op_compare };
            case op::bv_sge:     return { "bvsge", 2, 0, 1, false, smt_op_compare };
            case op::bv_sgt:     return { "bvsgt", 2, 0, 1, false, smt_op_compare };

            case op::fp_add:     return { "fp.add", 2, 0, 0, true, smt_op_other };
            case op::fp_sub:     return { "fp.sub", 2, 0, 0, true, smt_op_other };
            case op::fp_mul:     return { "fp.mul", 2, 0, 0, true, smt_op_other };
            case op::fp_div:     return { "fp.div", 2, 0, 0, true, smt_op_other };
            case op::fp_rem:     return { "fp.rem", 2, 0, 0, true, smt_op_other };

            case op::fp_true:    return { "fp.true",  2, 0, 1, true, smt_op_compare };
            case op::fp_false:   return { "fp.false", 2, 0, 1, true, smt_op_compare };
            case op::fp_ord:     return { "fp.ord",   2, 0, 1, true, smt_op_compare };
            case op::fp_une:     return { "fp.une",   2, 0, 1, true, smt_op_compare };
            case op::fp_one:     return { "fp.one",   2, 0, 1, true, smt_op_compare };
            case op::fp_uno:     return { "fp.uno",   2, 0, 1, true, smt_op_compare };
            case op::fp_oeq:     return { "fp.oeq",   2, 0, 1, true, smt_op_compare };
            case op::fp_ueq:     return { "fp.ueq",   2, 0, 1, true, smt_op_compare };

            case op::fp_ule:     return { "fp.ule",   2, 0, 1, true, smt_op_compare };
            case op::fp_ult:     return { "fp.ult",   2, 0, 1, true, smt_op_compare };
            case op::fp_uge:     return { "fp.uge",   2, 0, 1, true, smt_op_compare };
            case op::fp_ugt:     return { "fp.egt",   2, 0, 1, true, smt_op_compare };
            case op::fp_ole:     return { "fp.sle",   2, 0, 1, true, smt_op_compare };
            case op::fp_olt:     return { "fp.slt",   2, 0, 1, true, smt_op_compare };
            case op::fp_oge:     return { "fp.sge",   2, 0, 1, true, smt_op_compare };
            case op::fp_ogt:     return { "fp.sgt",   2, 0, 1, true, smt_op_compare };

            case op::bv_extract: return { "extract", 1, 2, 0, false, smt_op_other };
            case op::bv_concat:  return { "concat",  2, 0, 0, false, smt_op_other };

            case op::store:      return { "store", 2, 1, 0, false, smt_op_other };
            case op::load:       return { "load" , 1, 1, 0, false, smt_op_other };
        }

        return { "invalid",  0, 0, 0, false, smt_op_other };
    }

    template< typename T, smt_op id >
    constexpr bool smt_match_type()
    {
        return smt_traits( id ).is_float() == std::is_floating_point_v< T >;
    }

    template< smt_op_type t, typename T, smt_op id = smt_op::invalid >
    constexpr smt_op smt_match_op_f()
    {
        constexpr int bw = std::is_same_v< T, bool > ? 1 : sizeof( T ) * 8;
        if constexpr ( smt_traits( id ).type == t &&
                       smt_traits( id ).bitwidth == bw &&
                       smt_match_type< T, id >() )
            return id;
        else
            return smt_match_op_f< t, T, smt_op( int( id ) + 1 ) >();
    }

    template< smt_op_type t, typename T >
    constexpr smt_op smt_match_op = smt_match_op_f< t, T >();

    [[gnu::always_inline]] constexpr int smt_imm( smt_op op )   { return smt_traits( op ).imm_bytes; }
    [[gnu::always_inline]] constexpr int smt_bw( smt_op op )    { return smt_traits( op ).bitwidth; }
    [[gnu::always_inline]] constexpr int smt_arity( smt_op op ) { return smt_traits( op ).arity; }
    constexpr const char      *smt_name( smt_op op )  { return smt_traits( op ).name; }

    template< typename stream >
    auto operator<<( stream &o, smt_op op ) -> decltype( o << "" )
    {
        return o << smt_name( op );
    }

    template< typename type >
    constexpr size_t storage_size()
    {
        if constexpr ( std::is_same_v< type, void > )
            return 0;
        else
            return std::is_empty_v< type > ? 0 : sizeof( type );
    }
    
    template< typename T >
    void bitcast_mem( const uint8_t *mem, T &t )
    {
        memcpy( &t, mem, sizeof( T ) );
    }

    template< typename imm_t, int storage = storage_size< imm_t >() >
    struct smt_atom_t
    {
        smt_op  op;
        uint8_t _imm[ storage ];

        template< typename init_t >
        smt_atom_t( smt_op op, init_t init,
                    std::enable_if_t< std::is_convertible_v< init_t, imm_t >, bool > = false )
            : op( op )
        {
            if constexpr ( std::is_convertible_v< init_t, smt_varid_t > )
            {
                if ( smt_traits( op ).is_var() )
                    imm_set( smt_varid_t( init ) );
                else
                    imm_set( init );
            }
            else
            {
                assert( !smt_traits( op ).is_var() );
                imm_set( init );
            }
        }

        smt_atom_t( smt_op op ) : op( op ) {}
        smt_atom_t( const smt_atom_t & ) = delete;
        smt_atom_t( smt_atom_t && ) = delete;
        smt_atom_t &operator=( uint8_t op_ ) { op = smt_op( op_ ); }

        template< typename T >
        [[gnu::always_inline]] void copy_from( const smt_atom_t< T > &o, int size = 0 )
        {
            memcpy( this, &o, size ? size : o.size() );
        }

        bool is_const()   const { return smt_traits( op ).is_const(); }
        bool is_resize()  const { return smt_traits( op ).is_resize(); }
        bool is_cast()    const { return smt_traits( op ).is_cast(); }
        bool is_integral()    const { return smt_traits( op ).is_integral(); }
        bool is_float()    const { return smt_traits( op ).is_float(); }

        bool is_array()   const { return op == smt_op::array; }

        bool is_extract() const { return op == smt_op::bv_extract; }
        int  arity()      const { return smt_traits( op ).arity; }

        template< typename... args_t >
        static int first_( int v, args_t... )
        {
            return v;
        }

        static int first_() { return 0; }

        template< typename... args_t >
        int bw( args_t ... args ) const
        {
            if ( auto w = smt_bw( op ) ) /* fixed-width cases */
                return w;

                if ( op == smt_op::bv_concat )
            {
                assert( sizeof...( args ) == 2 );
                return ( 0 + ... + args );
            }

            if ( op == smt_op::bv_extract )
                return bounds().second - bounds().first + 1;

            if ( is_resize() )
                return imm_get< uint8_t >();

            // TODO store and load bw
            if ( is_array() )
                return imm_get< smt_array_type >().bitwidth;
            auto val = first_( args... );
            assert( ( ( val == args ) && ... ) );
            return val;
        }

        [[gnu::always_inline]] size_t size() const { return 1 + smt_imm( op ); }
        size_t imm_size() const { return smt_imm( op ); }
        smt_varid_t varid() const { return smt_traits( op ).is_var() ? imm_get< smt_varid_t >() : 0; }
        auto bounds() const { return imm_get< std::pair< uint8_t, uint8_t > >(); }
        auto array_type() const { return imm_get< smt_array_type >(); }

        uint64_t value() const
        {
            if ( bw() <= 8 )
                return imm_get< uint8_t >();
            if ( bw() <= 16 )
                return imm_get< uint16_t >();
            if ( bw() <= 32 )
                return imm_get< uint32_t >();
            if ( bw() <= 64 )
                return imm_get< uint64_t >();
            __builtin_unreachable(); // UNREACHABLE( "integral constant", *this, "too wide" );
        }

        template< typename T >
        T fpa_value() const
        {
            if constexpr ( std::is_same_v< T, float >  )
                return imm_get< float >();
            if constexpr ( std::is_same_v< T, double >  )
                return imm_get< double >();
            __builtin_unreachable(); // UNREACHABLE( "float constant", *this, "too wide" );
        }

        template< typename T > void imm_get( T &i ) const { bitcast_mem( _imm, i ); }
        template< typename T >    T imm_get() const { T i; imm_get( i ); return i; }
        template< typename T > void imm_set( T i )
        {
            assert( sizeof( i ) == smt_imm( op ) );
            memcpy( _imm, &i, sizeof( i ) );
        }

        template< typename stream >
        friend auto operator<<( stream &o, const smt_atom_t &a ) -> decltype( o << "" )
        {
            o << smt_name( a.op );

            if ( a.imm_size() )
            {
                if ( a.is_extract() ) {
                    const auto &b = a.bounds();
                    return o << '(' << b.first << ", " << b.second << ')';
                }
                if ( a.varid() )
                    return o << ' ' << a.varid();
                if ( a.is_const() )
                    return o << ' ' << a.value();
                if ( a.is_resize() )
                    return o << ' ' << a.bw();
            }

            return o;
        }
    };

    template< typename init_t >
    smt_atom_t( smt_op op, init_t init ) -> smt_atom_t< init_t >;

    using smt_atom = smt_atom_t< void >;
    static_assert( sizeof( smt_atom ) == 1 );

    template< template< typename... > class container_t, typename... container_args_t >
    struct smt_expr : container_t< uint8_t, container_args_t... >
    {
        using base = container_t< uint8_t, container_args_t... >;
        using typename base::size_type;
        using typename base::difference_type;
        using value_type = smt_atom;
        using reference = smt_atom &;
        using const_reference = const smt_atom &;

        template< typename val_t >
        struct proxy
        {
            val_t &_ref;
            proxy( val_t &r ) : _ref( r ) {}
            auto operator->() { return &_ref; }
        };

        template< typename base_t, typename val_t >
        struct iterator_t
        {
            base_t base;
            iterator_t( base_t b ) : base( b ) {}
            iterator_t &operator++() { base += ( *this )->size(); return *this; }
            iterator_t operator++( int ) { auto rv = *this; ++*this; return rv; }
            val_t &operator*() { return *reinterpret_cast< val_t * >( &*base ); }
            proxy< val_t > operator->() { return { **this }; }
            bool operator==( iterator_t o ) const { return base == o.base; }
            bool operator!=( iterator_t o ) const { return base != o.base; }
        };

        using const_iterator = iterator_t< typename base::const_iterator, const smt_atom >;
        using iterator       = iterator_t< typename base::iterator, smt_atom >;

        using base::base;

        iterator       begin()        { return base::begin(); }
        const_iterator begin()  const { return base::begin(); }
        const_iterator cbegin() const { return base::begin(); }
        iterator       end()          { return base::end(); }
        const_iterator end()    const { return base::end(); }
        const_iterator cend()   const { return base::end(); }

        iterator last( int off ) { return std::prev( end(), off + 1 ); }

        void apply() {}
        void apply( smt_op v ) { base::push_back( uint8_t( v ) ); }

        template< typename imm_t >
        void apply( const smt_atom_t< imm_t > &v )
        {
            int size = v.size();
            base::resize( base::size() + size );
            iterator it = base::end() - size;
            it->copy_from( v, size );
        }

        void apply( const smt_expr &e )
        {
            int e_size = e.base::size();
            base::resize( base::size() + e_size );
            auto it = base::end() - e_size;
            std::copy( e.base::begin(), e.base::end(), it );
        }

        template< typename arg1_t, typename arg2_t, typename... args_t >
        void apply( const arg1_t &arg_1, const arg2_t &arg_2, const args_t & ... args )
        {
            apply( arg_1 );
            apply( arg_2 );
            ( apply( args ) , ... );
        }

        template< typename stream >
        friend auto operator<<( stream &o, const smt_expr &e ) -> decltype( o << "" )
        {
            auto print = [&o, need_comma = false](const auto &a) mutable {
                if (need_comma) o << ", ";
                o << a;
                need_comma = true;
            };

            o << "(";
            for (const auto &a : e)
                print(a);
            return o << ")";
        }
    };

    
} // namespace __lava
