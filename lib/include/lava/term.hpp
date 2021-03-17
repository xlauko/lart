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
#include <lava/constant.hpp>

#include <set>
#include <stdio.h>
#include <type_traits>
#include <z3++.h>

#include <iostream>

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

    term_state_t state;
    
    struct z3_expr { 
        z3_expr( z3::expr e ) : expr( e ) {}
        /*{
            printf("assert: %s\n", Z3_ast_to_string(state.ctx, e));
        }*/
        z3::expr expr;
    };

    using term_storage = tagged_storage< z3_expr >;

    struct term : term_storage, domain_mixin< term >
    {
        using term_storage::term_storage;
        using base = domain_mixin< term >;

        using tr = const term &;
  
        term( z3::expr e ) : term_storage( z3_expr{ e } ) {}
        term( void *v, construct_shared_t s ) : term_storage( v, s ) {}

        template< typename type > static term lift( type value )
        {
            auto &ctx = state.ctx;

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
            constexpr auto digits = std::numeric_limits< type >::digits;
            char name[5 + digits] = {};
            std::strncpy( name, "var_", 4 );
            std::sprintf( name + 4, "%d", variable_counter() );

            auto &ctx = state.ctx;            
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

        static constant lower( tr ) { __builtin_unreachable(); };

        static void assume( tr t, bool expected ) 
        {
            auto &solver = state.solver;
            solver.add( expected ? t->expr : !t->expr );
            
            if ( solver.check() == z3::unsat ) {
                __lart_cancel();
            }
        }

        static tristate to_tristate( tr ) { return maybe; }

        /* arithmetic operations */
        static term op_add ( tr a, tr b ) { return a->expr + b->expr; }
        static term op_sub ( tr a, tr b ) { return a->expr - b->expr; }
        static term op_mul ( tr a, tr b ) { return a->expr * b->expr; }
        static term op_udiv( tr a, tr b ) { return z3::udiv( a->expr, b->expr ); }
        static term op_sdiv( tr a, tr b ) { return a->expr / b->expr; }
        static term op_urem( tr a, tr b ) { return z3::urem( a->expr, b->expr ); }
        static term op_srem( tr a, tr b ) { return z3::srem( a->expr, b->expr ); }

        /* bitwise operations */
        static term op_shl ( tr a, tr b ) { return z3::shl( a->expr, b->expr ); }
        static term op_lshr( tr a, tr b ) { return z3::lshr( a->expr, b->expr ); }
        static term op_ashr( tr a, tr b ) { return z3::ashr( a->expr, b->expr ); }
        static term op_and ( tr a, tr b ) { return a->expr & b->expr; }
        static term op_or  ( tr a, tr b ) { return a->expr | b->expr; }
        static term op_xor ( tr a, tr b ) { return a->expr ^ b->expr; }

        using bw = uint8_t;

        /* comparison operations */
        static term op_eq ( tr a, tr b ) { return a->expr == b->expr; }
        static term op_ne ( tr a, tr b ) { return a->expr != b->expr; }
        static term op_ugt( tr a, tr b ) { return z3::ugt( a->expr, b->expr ); }
        static term op_uge( tr a, tr b ) { return z3::uge( a->expr, b->expr ); }
        static term op_ult( tr a, tr b ) { return z3::ult( a->expr, b->expr ); }
        static term op_ule( tr a, tr b ) { return z3::ule( a->expr, b->expr ); }
        static term op_sgt( tr a, tr b ) { return a->expr > b->expr; }
        static term op_sge( tr a, tr b ) { return a->expr >= b->expr; }
        static term op_slt( tr a, tr b ) { return a->expr < b->expr; }
        static term op_sle( tr a, tr b ) { return a->expr <= b->expr; }

        // static term op_inttoptr( tr, bw ) { return {}; }
        // static term op_ptrtoint( tr, bw ) { return {}; }
        static term op_sext( tr t, bw b ) { return z3::sext( t->expr, b ); }
        // static term op_sitofp( tr, bw ) { return {}; }
        static term op_trunc( tr t, bw b ) { return t->expr.extract( b - 1, 0 ); }
        // static term op_uitofp( tr, bw ) { return {}; }
        static term op_zext( tr t, bw b ) { return z3::zext( t->expr, b ); }
        // static term op_zfit( tr t, bw ) { return {}; }


        static void dump( tr t )
        {
            printf( "%s\n", Z3_ast_to_string( state.ctx, t->expr ) );
        }

        static std::string trace( tr t )
        {
            return Z3_ast_to_string( state.ctx, t->expr );
        }
    };


} // namespace __lava
