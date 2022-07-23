/*
 * (c) 2021 Hugo Adamove <xadamove@mail.muni.cz>
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

#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <lava/support/base.hpp>
#include <lava/support/tristate.hpp>

namespace __lava
{
    struct [[gnu::packed]] float_storage
    {
        using underlying_type = uint8_t;

        enum value_type : underlying_type {
            neg = 0, nzero, pzero, pos, ninf, pinf, nan,
            zero, nzero_neg, pzero_pos, finite, top, bottom
        } value;

        constexpr float_storage() : value( top ) {}

        constexpr float_storage( value_type v ) : value( v ) {}
        constexpr float_storage( underlying_type v )
            : float_storage( value_type( v ) ) {}
    };

    // struct lattice
    // {
    //     using fl = float_storage;
    //     using fl_values = std::unordered_set< fl::value_type >;

    //     using edges = std::unordered_map< fl::value_type, fl_values >;


    //     /* TODO: replace edge map with lattice order matrix? */

    //     inline static edges above = {
    //         { fl::bottom,    { fl::ninf, fl::neg, fl::nzero, fl::pzero, fl::pos, fl::pinf, fl::nan } },
    //         { fl::ninf,      { fl::top } },
    //         { fl::neg,       { fl::nzero_neg } },
    //         { fl::nzero,     { fl::nzero_neg, fl::zero } },
    //         { fl::pzero,     { fl::pzero_pos, fl::zero } },
    //         { fl::pos,       { fl::pzero_pos } },
    //         { fl::pinf,      { fl::top } },
    //         { fl::nan,       { fl::top } },
    //         { fl::nzero_neg, { fl::finite } },
    //         { fl::zero,      { fl::finite } },
    //         { fl::pzero_pos, { fl::finite } },
    //         { fl::finite,    { fl::top } },
    //         { fl::top,       {} }
    //     };

    //     inline static edges below = {
    //         { fl::bottom,    {} },
    //         { fl::ninf,      { fl::bottom } },
    //         { fl::neg,       { fl::bottom } },
    //         { fl::nzero,     { fl::bottom } },
    //         { fl::pzero,     { fl::bottom } },
    //         { fl::pos,       { fl::bottom } },
    //         { fl::pinf,      { fl::bottom } },
    //         { fl::nan,       { fl::bottom } },
    //         { fl::nzero_neg, { fl::neg, fl::nzero } },
    //         { fl::zero,      { fl::nzero, fl::pzero } },
    //         { fl::pzero_pos, { fl::pzero, fl::pos } },
    //         { fl::finite,    { fl::nzero_neg, fl::zero, fl::pzero_pos } },
    //         { fl::top,       { fl::ninf, fl::finite, fl::pinf, fl::nan } }
    //     };

    //     static inline bool is_elementary( fl::value_type a )
    //     {
    //         return above[ fl::bottom ].count( a );
    //     }

    //     static fl::value_type supremum( fl::value_type a, fl::value_type b )
    //     {
    //         fl_values visited;
    //         std::vector< fl::value_type > stack_a{ a }, stack_b{ b };

    //         while ( !stack_a.empty() || !stack_b.empty() )
    //         {
    //             if ( visited.count( a ) )
    //                 return a;
    //             if ( visited.count( b ) )
    //                 return b;

    //             if ( !stack_a.empty() ) {
    //                 a = stack_a.back(); stack_a.pop_back();
    //             }

    //             if ( !stack_b.empty() ) {
    //                 b = stack_b.back(); stack_b.pop_back();
    //             }

    //             visited.insert( a );
    //             visited.insert( b );

    //             stack_a.insert( stack_a.begin(), above[ a ].begin(), above[ a ].end() );
    //             stack_b.insert( stack_b.begin(), above[ b ].begin(), above[ b ].end() );
    //         }

    //         return fl::top;
    //     }

    //     static fl_values decompose( fl::value_type a )
    //     {
    //         fl_values result;
    //         std::vector< fl::value_type > stack;

    //         stack.push_back( a );
    //         while ( !stack.empty() )
    //         {
    //             auto v = stack.back();
    //             stack.pop_back();

    //             if ( is_elementary( v ) )
    //                 result.insert( v );
    //             else
    //                 stack.insert( stack.begin(), below[ v ].begin(), below[ v ].end() );
    //         }

    //         return result;
    //     }
    // };

    template< template< typename > typename storage >
    struct floats : storage< float_storage >
                 , domain_mixin< floats< storage > >
    {

        using fl = float_storage;
        using fv = floats;
        using fr = const floats &;

        using base = storage< float_storage >;
        using mixin = domain_mixin< floats >;

        using base::base;

        floats( fl::value_type value ) : storage< fl >( fl{ value } ) {}
        floats( fr o ) : floats( o->value ) {}

        floats( void *v, __lava::construct_shared_t s )
            : storage< fl >( v, s )
        {}

        bool is_bottom() const { return value() == fl::bottom; }
        bool is_finite() const { return value() <= fl::finite; }
        bool is_inf() const { return value() == fl::pinf || value() == fl::ninf; }

        template< typename type >
        static fv lift( type v )
        {
            if constexpr ( std::is_integral_v< type > )
            {
                if ( v == 0 )
                    return { fl::zero };
                return { ( v > 0 ) ? fl::pos : fl::neg };
            }
            // TODO: add signbit => classify -Z and +Z
            else if constexpr ( std::is_floating_point_v< type > )
            {
                switch ( std::fpclassify( v ) )
                {
                    case FP_INFINITE:  return { ( v > 0 ) ? fl::pinf : fl::ninf };
                    case FP_SUBNORMAL: /* same as FP_NORMAL */
                    case FP_NORMAL:    return { ( v > 0 ) ? fl::pos : fl::neg };
                    case FP_ZERO:      return { fl::zero };
                    default: mixin::fail( "unable to lift value" );
                }
            }
            else
                mixin::fail( "unable to lift value" );

            __builtin_unreachable();
        }

        template< typename type > static floats any() { return { fl::top }; }

        fl::value_type  value() const { return this->get().value; }
        fl::value_type &value()       { return this->get().value; }

        void intersect( fl::value_type b )
        {
            fl::value_type &v = value();
            if ( ( v == fl::finite && b <= fl::finite ) || v == fl::top )
                v = b;
            else if ( v != b && b != fl::finite && b != fl::top )
                v = fl::bottom;

            if ( this->is_bottom() )
                __lart_cancel();
        }

        static void assume( floats &f, bool constraint )
        {
            f.intersect( constraint ? fl::pos : fl::pzero );
        }

        static tristate to_tristate( fr f ) { return { tristate::value_t(f->value) }; }


        using elementary_op = fl::value_type[ 7 ][ 7 ];

        static constexpr fl::value_type add[ 7 ][ 7 ] {
            { fl::neg,    fl::neg,   fl::neg,   fl::finite, fl::ninf,  fl::pinf,  fl::nan },
            { fl::neg,    fl::nzero, fl::pzero, fl::pos,    fl::ninf,  fl::pinf,  fl::nan },
            { fl::neg,    fl::pzero, fl::pzero, fl::pos,    fl::ninf,  fl::pinf,  fl::nan },
            { fl::finite, fl::pos,   fl::pos,   fl::pos,    fl::ninf,  fl::pinf,  fl::nan },
            { fl::ninf,   fl::ninf,  fl::ninf,  fl::ninf,   fl::ninf,  fl::nan,   fl::nan },
            { fl::pinf,   fl::pinf,  fl::pinf,  fl::pinf,   fl::nan,   fl::pinf,  fl::nan },
            { fl::nan,    fl::nan,   fl::nan,   fl::nan,    fl::nan,   fl::nan,   fl::nan }
        };

        static constexpr fl::value_type sub[ 7 ][ 7 ] {
            { fl::finite, fl::neg,   fl::neg,   fl::neg,    fl::pinf, fl::ninf, fl::nan },
            { fl::pos,    fl::pzero, fl::nzero, fl::neg,    fl::pinf, fl::ninf, fl::nan },
            { fl::pos,    fl::pzero, fl::pzero, fl::neg,    fl::pinf, fl::ninf, fl::nan },
            { fl::pos,    fl::pos,   fl::pos,   fl::finite, fl::pinf, fl::ninf, fl::nan },
            { fl::ninf,   fl::ninf,  fl::ninf,  fl::ninf,   fl::nan,  fl::ninf, fl::nan },
            { fl::pinf,   fl::pinf,  fl::pinf,  fl::pinf,   fl::pinf, fl::nan,  fl::nan },
            { fl::nan,    fl::nan,   fl::nan,   fl::nan,    fl::nan,  fl::nan,  fl::nan }
        };

        static constexpr fl::value_type mul[ 7 ][ 7 ] {
            { fl::pos,   fl::pzero, fl::nzero, fl::neg,   fl::pinf, fl::ninf, fl::nan },
            { fl::pzero, fl::pzero, fl::nzero, fl::nzero, fl::nan,  fl::nan,  fl::nan },
            { fl::nzero, fl::nzero, fl::pzero, fl::pzero, fl::nan,  fl::nan,  fl::nan },
            { fl::neg,   fl::nzero, fl::pzero, fl::pos,   fl::ninf, fl::pinf, fl::nan },
            { fl::pinf,  fl::nan,   fl::nan,   fl::ninf,  fl::pinf, fl::ninf, fl::nan },
            { fl::ninf,  fl::nan,   fl::nan,   fl::pinf,  fl::ninf, fl::pinf, fl::nan },
            { fl::nan,   fl::nan,   fl::nan,   fl::nan,   fl::nan,  fl::nan,  fl::nan }
        };

        static constexpr fl::value_type div[ 7 ][ 7 ] {
            { fl::pos,   fl::pinf,  fl::ninf, fl::neg,   fl::pzero, fl::nzero, fl::nan },
            { fl::pzero, fl::nan,   fl::nan,  fl::nzero, fl::pzero, fl::nzero, fl::nan },
            { fl::nzero, fl::nan,   fl::nan,  fl::pzero, fl::nzero, fl::pzero, fl::nan },
            { fl::neg,   fl::ninf,  fl::pinf, fl::pos,   fl::nzero, fl::pzero, fl::nan },
            { fl::pinf,  fl::pinf,  fl::ninf, fl::ninf,  fl::nan,   fl::nan,   fl::nan },
            { fl::ninf,  fl::ninf,  fl::pinf, fl::pinf,  fl::nan,   fl::nan,   fl::nan },
            { fl::nan,   fl::nan,   fl::nan,  fl::nan,   fl::nan,   fl::nan,   fl::nan }
        };

        static constexpr fl::value_type rem[ 7 ][ 7 ] {
            { fl::finite, fl::nan,   fl::nan, fl::finite, fl::neg,   fl::neg,   fl::nan },
            { fl::nzero,  fl::nan,   fl::nan, fl::nzero,  fl::nzero, fl::nzero, fl::nan },
            { fl::pzero,  fl::nan,   fl::nan, fl::pzero,  fl::pzero, fl::pzero, fl::nan },
            { fl::finite, fl::nan,   fl::nan, fl::finite, fl::pos,   fl::pos,   fl::nan },
            { fl::nan,    fl::nan,   fl::nan, fl::nan,    fl::nan,   fl::nan,   fl::nan },
            { fl::nan,    fl::nan,   fl::nan, fl::nan,    fl::nan,   fl::nan,   fl::nan },
            { fl::nan,    fl::nan,   fl::nan, fl::nan,    fl::nan,   fl::nan,   fl::nan }
        };

        static constexpr fl::value_type foeq[ 7 ][ 7 ] {
            { fl::finite, fl::pzero, fl::pzero, fl::pzero,  fl::pzero, fl::pzero, fl::pzero },
            { fl::pzero,  fl::pos,   fl::pos,   fl::pzero,  fl::pzero, fl::pzero, fl::pzero },
            { fl::pzero,  fl::pos,   fl::pos,   fl::pzero,  fl::pzero, fl::pzero, fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::finite, fl::pzero, fl::pzero, fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::pzero,  fl::pos,   fl::pzero, fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::pzero,  fl::pzero, fl::pos,   fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::pzero,  fl::pzero, fl::pzero, fl::pzero }
        };

        static constexpr fl::value_type fone[ 7 ][ 7 ] {
            { fl::finite, fl::pos,   fl::pos,   fl::pos,    fl::pos,   fl::pos,   fl::pos },
            { fl::pos,    fl::pzero, fl::pzero, fl::pos,    fl::pos,   fl::pos,   fl::pos },
            { fl::pos,    fl::pzero, fl::pzero, fl::pos,    fl::pos,   fl::pos,   fl::pos },
            { fl::pos,    fl::pos,   fl::pos,   fl::finite, fl::pos,   fl::pos,   fl::pos },
            { fl::pos,    fl::pos,   fl::pos,   fl::pos,    fl::pzero, fl::pos,   fl::pos },
            { fl::pos,    fl::pos,   fl::pos,   fl::pos,    fl::pos,   fl::pzero, fl::pos },
            { fl::pos,    fl::pos,   fl::pos,   fl::pos,    fl::pos,   fl::pos,   fl::pos }
        };

        static constexpr fl::value_type fogt[ 7 ][ 7 ] {
            { fl::finite, fl::pzero, fl::pzero, fl::pzero,  fl::pos,   fl::pzero, fl::pzero },
            { fl::pos,    fl::pzero, fl::pzero, fl::pzero,  fl::pos,   fl::pzero, fl::pzero },
            { fl::pos,    fl::pzero, fl::pzero, fl::pzero,  fl::pos,   fl::pzero, fl::pzero },
            { fl::pos,    fl::pos,   fl::pos,   fl::finite, fl::pos,   fl::pzero, fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::pzero,  fl::pzero, fl::pzero, fl::pzero },
            { fl::pos,    fl::pos,   fl::pos,   fl::pos,    fl::pos,   fl::pzero, fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::pzero,  fl::pzero, fl::pzero, fl::pzero }
        };

        static constexpr fl::value_type foge[ 7 ][ 7 ] {
            { fl::finite, fl::pzero, fl::pzero, fl::pzero,  fl::pos,   fl::pzero, fl::pzero },
            { fl::pos,    fl::pos,   fl::pos,   fl::pzero,  fl::pos,   fl::pzero, fl::pzero },
            { fl::pos,    fl::pos,   fl::pos,   fl::pzero,  fl::pos,   fl::pzero, fl::pzero },
            { fl::pos,    fl::pos,   fl::pos,   fl::finite, fl::pos,   fl::pzero, fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::pzero,  fl::pos,   fl::pzero, fl::pzero },
            { fl::pos,    fl::pos,   fl::pos,   fl::pos,    fl::pos,   fl::pos,   fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::pzero,  fl::pzero, fl::pzero, fl::pzero }
        };

        static constexpr fl::value_type folt[ 7 ][ 7 ] {
            { fl::finite, fl::pos,   fl::pos,   fl::pos,    fl::pzero, fl::pos,   fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::pos,    fl::pzero, fl::pos,   fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::pos,    fl::pzero, fl::pos,   fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::finite, fl::pzero, fl::pos,   fl::pzero },
            { fl::pos,    fl::pos,   fl::pos,   fl::pos,    fl::pzero, fl::pos,   fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::pzero,  fl::pzero, fl::pzero, fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::pzero,  fl::pzero, fl::pzero, fl::pzero }
        };

        static constexpr fl::value_type fole[ 7 ][ 7 ] {
            { fl::finite, fl::pos,   fl::pos,   fl::pos,    fl::pzero, fl::pos,   fl::pzero },
            { fl::pzero,  fl::pos,   fl::pos,   fl::pos,    fl::pzero, fl::pos,   fl::pzero },
            { fl::pzero,  fl::pos,   fl::pos,   fl::pos,    fl::pzero, fl::pos,   fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::finite, fl::pzero, fl::pos,   fl::pzero },
            { fl::pos,    fl::pos,   fl::pos,   fl::pos,    fl::pos,   fl::pos,   fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::pzero,  fl::pzero, fl::pos,   fl::pzero },
            { fl::pzero,  fl::pzero, fl::pzero, fl::pzero,  fl::pzero, fl::pzero, fl::pzero }
        };

        template< typename op_t >
        static fl::value_type eval(op_t /* op */, fl::value_type /* a */, fl::value_type /* b */ )
        {
            fl::value_type res = fl::bottom;

            // for ( auto &v : lattice::decompose( a ) )
            //     for ( auto &u : lattice::decompose( b ) )
            //         res = lattice::supremum( res, *op[ u ][ v ] );

            return res;
        }

        // Binary ops
        static fv op_fadd( fr a, fr b ) { return { eval( &add, a->value, b->value ) }; }
        static fv op_fsub( fr a, fr b ) { return { eval( &sub, a->value, b->value ) }; }
        static fv op_fmul( fr a, fr b ) { return { eval( &mul, a->value, b->value ) }; }
        static fv op_fdiv( fr a, fr b ) { return { eval( &div, a->value, b->value ) }; }
        static fv op_frem( fr a, fr b ) { return { eval( &rem, a->value, b->value ) }; }

        // Comparison ops
        static fv op_foeq( fr a, fr b ) { return { eval( &foeq, a->value, b->value ) }; }
        static fv op_fone( fr a, fr b ) { return { eval( &fone, a->value, b->value ) }; }
        static fv op_fogt( fr a, fr b ) { return { eval( &fogt, a->value, b->value ) }; }
        static fv op_foge( fr a, fr b ) { return { eval( &foge, a->value, b->value ) }; }
        static fv op_folt( fr a, fr b ) { return { eval( &folt, a->value, b->value ) }; }
        static fv op_fole( fr a, fr b ) { return { eval( &fole, a->value, b->value ) }; }

        static const char *str( fr a )
        {
            switch ( a->value ) {
                case fl::neg:       return "neg";
                case fl::nzero:     return "-zero";
                case fl::pzero:     return "+zero";
                case fl::pos:       return "pos";
                case fl::ninf:      return "-inf";
                case fl::pinf:      return "+inf";
                case fl::nan:       return "nan";
                case fl::zero:      return "zero";
                case fl::nzero_neg: return "-zero & neg";
                case fl::pzero_pos: return "+zero & pos";
                case fl::finite:    return "finite";
                case fl::top:       return "⊤";
                case fl::bottom:    return "⊥";
            }

            __builtin_unreachable();
        }

        static void dump( fr a )
        {
            printf( "%s\n", str( a ) );
        }

        static std::string trace( fr a ) { return str( a ); }

        template< typename stream >
        friend stream& operator<<( stream &os, fr a )
        {
            return os << str( a );
        }
    };

} // namespace __lava
