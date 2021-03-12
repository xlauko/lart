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
#include <z3++.h>

using namespace std::string_literals;

namespace __lava
{
    using term_storage = tagged_storage< z3::expr >;
    
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

    struct term : term_storage, domain_mixin< term >
    {
        using term_storage::term_storage;
        using base = domain_mixin< term >;

        using tr = const term &;
  
        term() : term_storage( state.ctx ) {}
        term( void *v, construct_shared_t s ) : term_storage( v, s ) {}

        template< typename type > static term lift( type ) { return z3::expr( state.ctx ); }
        template< typename type > static term any()
        {
            if constexpr ( std::is_integral_v < type > )
            {
                constexpr auto digits = std::numeric_limits< type >::digits;
                char name[5 + digits] = {};
                std::strncpy( name, "var_", 4 );
                std::sprintf( name + 4, "%d", variable_counter() );
                return state.ctx.bv_const( name, bitwidth_v< type > );
            }
            __builtin_unreachable();
        }

        static constant lower( tr ) { return {}; };

        static void assume( tr, bool ) {}

        static tristate to_tristate( tr ) { return maybe; }

        /* arithmetic operations */
        static term op_add ( tr, tr ) { return {}; }
        static term op_fadd( tr, tr ) { return {}; }
        static term op_sub ( tr, tr ) { return {}; }
        static term op_fsub( tr, tr ) { return {}; }
        static term op_mul ( tr, tr ) { return {}; }
        static term op_fmul( tr, tr ) { return {}; }
        static term op_udiv( tr, tr ) { return {}; }
        static term op_sdiv( tr, tr ) { return {}; }
        static term op_fdiv( tr, tr ) { return {}; }
        static term op_urem( tr, tr ) { return {}; }
        static term op_srem( tr, tr ) { return {}; }
        static term op_frem( tr, tr ) { return {}; }

        static term op_fneg( tr ) { return {}; }

        /* bitwise operations */
        static term op_shl ( tr, tr ) { return {}; }
        static term op_lshr( tr, tr ) { return {}; }
        static term op_ashr( tr, tr ) { return {}; }
        static term op_and ( tr, tr ) { return {}; }
        static term op_or  ( tr, tr ) { return {}; }
        static term op_xor ( tr, tr ) { return {}; }

        using bw = uint8_t;

        /* comparison operations */
        static term op_eq ( tr, tr ) { return {}; }
        static term op_ne ( tr, tr ) { return {}; }
        static term op_ugt( tr, tr ) { return {}; }
        static term op_uge( tr, tr ) { return {}; }
        static term op_ult( tr, tr ) { return {}; }
        static term op_ule( tr, tr ) { return {}; }
        static term op_sgt( tr, tr ) { return {}; }
        static term op_sge( tr, tr ) { return {}; }
        static term op_slt( tr, tr ) { return {}; }
        static term op_sle( tr, tr ) { return {}; }

        static term op_ffalse( tr, tr ) { return {}; }
        static term op_ftrue( tr, tr ) { return {}; }


        static term op_fpext( tr, bw ) { return {}; }
        static term op_fptosi( tr, bw ) { return {}; }
        static term op_fptoui( tr, bw ) { return {}; }
        static term op_fptrunc( tr, bw ) { return {}; }
        static term op_inttoptr( tr, bw ) { return {}; }
        static term op_ptrtoint( tr, bw ) { return {}; }
        static term op_sext( tr, bw ) { return {}; }
        static term op_sitofp( tr, bw ) { return {}; }
        static term op_trunc( tr, bw ) { return {}; }
        static term op_uitofp( tr, bw ) { return {}; }
        static term op_zext( tr, bw ) { return {}; }
        static term op_zfit( tr, bw ) { return {}; }

        static std::string trace( tr ) { return "term"; }
    };


} // namespace __lava
