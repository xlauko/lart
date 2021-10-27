/*
 * (c) 2020 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <lava/support/base.hpp>
#include <lava/support/tristate.hpp>

namespace __lava
{
    struct [[gnu::packed]] expr_storage {};
    
    template< template< typename > typename storage >
    struct expr : storage< expr_storage > 
                , domain_mixin< expr< storage > >
    {
        using base = storage< expr_storage >;
        using mixin = domain_mixin< expr >;

        using bw = typename mixin::bw;
        using base::base;

        using ev = expr;
        using er = const expr &;

        template< typename type > static expr lift( const type& ) { return {}; }
        template< typename type > static expr any() { return {}; }

        template< typename size >
        static expr op_alloca( const size&, uint8_t ) { return {}; }

        static expr op_load( er, uint8_t ) { return {}; }
        static expr op_load_at( er, er, uint8_t ) { return {}; }

        template< typename scalar >
        static void op_store( er, const scalar&, uint8_t ) {}

        static void assume( er, bool ) {}

        static tristate to_tristate( er ) { return maybe; }

        /* arithmetic operations */
        static expr op_add ( er, er ) { return {}; }
        static expr op_fadd( er, er ) { return {}; }
        static expr op_sub ( er, er ) { return {}; }
        static expr op_fsub( er, er ) { return {}; }
        static expr op_mul ( er, er ) { return {}; }
        static expr op_fmul( er, er ) { return {}; }
        static expr op_udiv( er, er ) { return {}; }
        static expr op_sdiv( er, er ) { return {}; }
        static expr op_fdiv( er, er ) { return {}; }
        static expr op_urem( er, er ) { return {}; }
        static expr op_srem( er, er ) { return {}; }
        static expr op_frem( er, er ) { return {}; }

        static expr op_fneg( er ) { return {}; }

        /* bitwise operations */
        static expr op_shl ( er, er ) { return {}; }
        static expr op_lshr( er, er ) { return {}; }
        static expr op_ashr( er, er ) { return {}; }
        static expr op_and ( er, er ) { return {}; }
        static expr op_or  ( er, er ) { return {}; }
        static expr op_xor ( er, er ) { return {}; }

        /* comparison operations */
        static expr op_foeq( er, er ) { return {}; }
        static expr op_fogt( er, er ) { return {}; }
        static expr op_foge( er, er ) { return {}; }
        static expr op_folt( er, er ) { return {}; }
        static expr op_fole( er, er ) { return {}; }
        static expr op_fone( er, er ) { return {}; }
        static expr op_ford( er, er ) { return {}; }
        static expr op_funo( er, er ) { return {}; }
        static expr op_fueq( er, er ) { return {}; }
        static expr op_fugt( er, er ) { return {}; }
        static expr op_fuge( er, er ) { return {}; }
        static expr op_fult( er, er ) { return {}; }
        static expr op_fule( er, er ) { return {}; }
        static expr op_fune( er, er ) { return {}; }

        static expr op_eq ( er, er ) { return {}; }
        static expr op_ne ( er, er ) { return {}; }
        static expr op_ugt( er, er ) { return {}; }
        static expr op_uge( er, er ) { return {}; }
        static expr op_ult( er, er ) { return {}; }
        static expr op_ule( er, er ) { return {}; }
        static expr op_sgt( er, er ) { return {}; }
        static expr op_sge( er, er ) { return {}; }
        static expr op_slt( er, er ) { return {}; }
        static expr op_sle( er, er ) { return {}; }

        static expr op_ffalse( er, er ) { return {}; }
        static expr op_ftrue( er, er ) { return {}; }


        static expr op_fpext( er, bw ) { return {}; }
        static expr op_fptosi( er, bw ) { return {}; }
        static expr op_fptoui( er, bw ) { return {}; }
        static expr op_fptrunc( er, bw ) { return {}; }
        static expr op_inttoptr( er, bw ) { return {}; }
        static expr op_ptrtoint( er, bw ) { return {}; }
        static expr op_sext( er, bw ) { return {}; }
        static expr op_sitofp( er, bw ) { return {}; }
        static expr op_trunc( er, bw ) { return {}; }
        static expr op_uitofp( er, bw ) { return {}; }
        static expr op_zext( er, bw ) { return {}; }
        static expr op_zfit( er, bw ) { return {}; }

        static void dump( er ) { printf( "expr\n" ); }
        static std::string trace( er ) { return "expr"; }
        
        template< typename stream >
        friend stream& operator<<( stream &os, er ) { return os << "expr"; }
    };

} // namespace __lava