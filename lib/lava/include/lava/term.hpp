/*
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

#include <lava/support/tristate.hpp>
#include <lava/support/base.hpp>

#include <cstdio>
#include <cstring>
#include <type_traits>
#include <z3++.h>

using namespace std::string_literals;

namespace __lava
{
    struct term_state_t
    {
        term_state_t()
            : solver( ctx )
        {}

        z3::context ctx;
        z3::solver solver;
    };

    static unsigned variable_counter()
    {
        static unsigned v = 0;
        return ++v;
    }

    term_state_t __term_state;
    
    template< template< typename > typename storage >
    struct term : storage< z3::expr >
                , domain_mixin< term< storage > >
    {
        using base = storage< z3::expr >;
        using mixin = domain_mixin< term >;

        using bw = typename mixin::bw;
        using base::base;

        using tr = const term &;

        template< typename type > static term lift( const type &value )
        {
            auto &ctx = __term_state.ctx;

            if constexpr ( std::is_same_v< type, bool > )
            {
                return ctx.bool_val( value );
            }

            if constexpr ( std::is_integral_v < type > )
            {
                return ctx.bv_val( value, bitwidth_v< type > );
            }

            __builtin_unreachable();
        }

        template< typename type > static term any()
        {
            constexpr auto digits = std::numeric_limits< unsigned >::digits;
            char name[5 + digits] = {};
            std::strncpy( name, "var_", 4 );
            std::sprintf( name + 4, "%u", variable_counter() );

            auto &ctx = __term_state.ctx;            
            if constexpr ( std::is_integral_v < type > )
            {
                return ctx.bv_const( name, bitwidth_v< type > );
            }

            if constexpr ( std::is_floating_point_v< type > )
            {
                if constexpr ( bitwidth_v< type > == 32 )
                    return ctx.fpa_const( name, 8, 24 );
                if constexpr ( bitwidth_v< type > == 64 )
                    return ctx.fpa_const( name, 11, 53 );
            }
            __builtin_unreachable();
        }

        static z3::expr tobool( const z3::expr &e )
        {
            assert( e.is_bv() );
            assert( e.get_sort().bv_size() == 1 );
            return e == __term_state.ctx.bv_val( 1, 1 );
        }

        static void assume( tr t, bool expected ) 
        {
            auto &solver = __term_state.solver;
            const auto& e = t.get();
            auto b = e.is_bool() ? e : tobool( e );
            solver.add( expected ? b : !b );
            
            if ( solver.check() == z3::unsat ) {
                __lart_cancel();
            }
        }

        static tristate to_tristate( tr ) { return maybe; }

        /* arithmetic operations */
        static term op_add ( tr a, tr b ) { return a.get() + b.get(); }
        static term op_sub ( tr a, tr b ) { return a.get() - b.get(); }
        static term op_mul ( tr a, tr b ) { return a.get() * b.get(); }
        static term op_udiv( tr a, tr b ) { return z3::udiv( a.get(), b.get() ); }
        static term op_sdiv( tr a, tr b ) { return a.get() / b.get(); }
        static term op_urem( tr a, tr b ) { return z3::urem( a.get(), b.get() ); }
        static term op_srem( tr a, tr b ) { return z3::srem( a.get(), b.get() ); }

        /* bitwise operations */
        static term op_shl ( tr a, tr b ) { return z3::shl( a.get(), b.get() ); }
        static term op_lshr( tr a, tr b ) { return z3::lshr( a.get(), b.get() ); }
        static term op_ashr( tr a, tr b ) { return z3::ashr( a.get(), b.get() ); }
        static term op_and ( tr a, tr b ) { return a.get() & b.get(); }
        static term op_or  ( tr a, tr b ) { return a.get() | b.get(); }
        static term op_xor ( tr a, tr b ) { return a.get() ^ b.get(); }

        /* comparison operations */
        static term op_eq ( tr a, tr b ) { return a.get() == b.get(); }
        static term op_ne ( tr a, tr b ) { return a.get() != b.get(); }
        static term op_ugt( tr a, tr b ) { return z3::ugt( a.get(), b.get() ); }
        static term op_uge( tr a, tr b ) { return z3::uge( a.get(), b.get() ); }
        static term op_ult( tr a, tr b ) { return z3::ult( a.get(), b.get() ); }
        static term op_ule( tr a, tr b ) { return z3::ule( a.get(), b.get() ); }
        static term op_sgt( tr a, tr b ) { return a.get() > b.get(); }
        static term op_sge( tr a, tr b ) { return a.get() >= b.get(); }
        static term op_slt( tr a, tr b ) { return a.get() < b.get(); }
        static term op_sle( tr a, tr b ) { return a.get() <= b.get(); }

        // static term op_inttoptr( tr, bw ) { return {}; }
        // static term op_ptrtoint( tr, bw ) { return {}; }
        static term op_sext( tr t, bw b ) { return z3::sext( t.get(), b ); }
        // static term op_sitofp( tr, bw ) { return {}; }
        static term op_trunc( tr t, bw b ) { return t.get().extract( b - 1, 0 ); }
        // static term op_uitofp( tr, bw ) { return {}; }
        static term op_zext( tr t, bw b ) { 
            auto &v = t.get();
            return z3::zext( v, b - v.get_sort().bv_size() );
        }
        // static term op_zfit( tr t, bw ) { return {}; }

        static void dump( tr t )
        {
            printf( "%s\n", Z3_ast_to_string( __term_state.ctx, t.get() ) );
        }

        static std::string trace( tr t )
        {
            return Z3_ast_to_string( __term_state.ctx, t.get() );
        }
    };


} // namespace __lava
