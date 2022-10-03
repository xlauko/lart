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
#include <lava/support/smt.hpp>
#include <lava/support/tristate.hpp>

#include <runtime/vector.hpp>

namespace __lava
{
    struct [[gnu::packed]] expr_storage
    {
        expr_storage() = default;
        expr_storage(const expr_storage &) = default;
        expr_storage(expr_storage &&) = default;

        template< typename imm_t >
        expr_storage( const smt_atom_t< imm_t > &a )
        {
            rpn.apply(a);
        }

        template< typename... args_t  >
        expr_storage( const args_t &... args )
        {
            ( rpn.apply( args ) , ... );
        }

        smt_expr< __lart::rt::vector > rpn;
    };

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

        using op = smt_op;

        static int counter()
        {
            static int v = 0;
            return ++v;
        }

        static uint16_t array_counter()
        {
            static uint16_t v = 0;
            return ++v;
        }

        template< typename T >
        static expr lift( T value )
        {
            static_assert( std::is_arithmetic_v< T > || std::is_pointer_v< T > );
            constexpr auto op = smt_match_op< smt_op_const, T >;
            return { smt_atom_t< T >( op, value ) };
        }

        static expr lift( __lava::array_ref ) { mixin::fail("unsupported array lift"); }

        template< typename T >
        static expr any()
        {
            constexpr auto op = smt_match_op< smt_op_var, T >;
            return { smt_atom_t< smt_varid_t >( op, counter() ) };
        }

        static ev cast( er arg, bw w, op o )
        {
            smt_atom_t< uint8_t > atom( o, w );
            return { arg->rpn, atom };
        }

        template< typename size >
        static expr op_alloca( const size&, bw w )
        {
            using array = smt_array_type;

            // TODO get type as argument
            array type{ array_counter(), w, array::type_t::bitvector };
            return { smt_atom_t( op::array, type ) };
        }

        static void assume( er, bool ) {}

        static tristate to_tristate( er ) { return maybe; }

        /* arithmetic operations */
        static ev op_add ( er a, er b ) { return { a->rpn, b->rpn, op::bv_add  }; }
        static ev op_sub ( er a, er b ) { return { a->rpn, b->rpn, op::bv_sub  }; }
        static ev op_mul ( er a, er b ) { return { a->rpn, b->rpn, op::bv_mul  }; }
        static ev op_sdiv( er a, er b ) { return { a->rpn, b->rpn, op::bv_sdiv }; }
        static ev op_udiv( er a, er b ) { return { a->rpn, b->rpn, op::bv_udiv }; }
        static ev op_srem( er a, er b ) { return { a->rpn, b->rpn, op::bv_srem }; }
        static ev op_urem( er a, er b ) { return { a->rpn, b->rpn, op::bv_urem }; }

        static ev op_fadd( er a, er b ) { return { a->rpn, b->rpn, op::fp_add }; }
        static ev op_fsub( er a, er b ) { return { a->rpn, b->rpn, op::fp_sub }; }
        static ev op_fmul( er a, er b ) { return { a->rpn, b->rpn, op::fp_mul }; }
        static ev op_fdiv( er a, er b ) { return { a->rpn, b->rpn, op::fp_div }; }
        static ev op_frem( er a, er b ) { return { a->rpn, b->rpn, op::fp_rem }; }

        static ev op_not( er a ) { return { a, op::bv_not }; }
        static ev op_neg( er a ) { return { a, op::bv_neg }; }

        /* bitwise operations */
        static ev op_shl ( er a, er b ) { return { a->rpn, b->rpn, op::bv_shl  }; }
        static ev op_ashr( er a, er b ) { return { a->rpn, b->rpn, op::bv_ashr }; }
        static ev op_lshr( er a, er b ) { return { a->rpn, b->rpn, op::bv_lshr }; }
        static ev op_and ( er a, er b ) { return { a->rpn, b->rpn, op::bv_and  }; }
        static ev op_or  ( er a, er b ) { return { a->rpn, b->rpn, op::bv_or   }; }
        static ev op_xor ( er a, er b ) { return { a->rpn, b->rpn, op::bv_xor  }; }

        /* comparison operations */
        static ev op_eq ( er a, er b ) { return { a->rpn, b->rpn, op::eq }; };
        static ev op_ne ( er a, er b ) { return { a->rpn, b->rpn, op::neq }; };
        static ev op_ugt( er a, er b ) { return { a->rpn, b->rpn, op::bv_ugt }; };
        static ev op_uge( er a, er b ) { return { a->rpn, b->rpn, op::bv_uge }; };
        static ev op_ult( er a, er b ) { return { a->rpn, b->rpn, op::bv_ult }; };
        static ev op_ule( er a, er b ) { return { a->rpn, b->rpn, op::bv_ule }; };
        static ev op_sgt( er a, er b ) { return { a->rpn, b->rpn, op::bv_sgt }; };
        static ev op_sge( er a, er b ) { return { a->rpn, b->rpn, op::bv_sge }; };
        static ev op_slt( er a, er b ) { return { a->rpn, b->rpn, op::bv_slt }; };
        static ev op_sle( er a, er b ) { return { a->rpn, b->rpn, op::bv_sle }; };

        static ev op_foeq( er a, er b ) { return { a->rpn, b->rpn, op::fp_oeq }; }
        static ev op_fogt( er a, er b ) { return { a->rpn, b->rpn, op::fp_ogt }; }
        static ev op_foge( er a, er b ) { return { a->rpn, b->rpn, op::fp_oge }; }
        static ev op_folt( er a, er b ) { return { a->rpn, b->rpn, op::fp_olt }; }
        static ev op_fole( er a, er b ) { return { a->rpn, b->rpn, op::fp_ole }; }
        static ev op_fone( er a, er b ) { return { a->rpn, b->rpn, op::fp_one }; }
        static ev op_ford( er a, er b ) { return { a->rpn, b->rpn, op::fp_ord }; }
        static ev op_funo( er a, er b ) { return { a->rpn, b->rpn, op::fp_uno }; }
        static ev op_fueq( er a, er b ) { return { a->rpn, b->rpn, op::fp_ueq }; }
        static ev op_fugt( er a, er b ) { return { a->rpn, b->rpn, op::fp_ugt }; }
        static ev op_fuge( er a, er b ) { return { a->rpn, b->rpn, op::fp_uge }; }
        static ev op_fult( er a, er b ) { return { a->rpn, b->rpn, op::fp_ult }; }
        static ev op_fule( er a, er b ) { return { a->rpn, b->rpn, op::fp_ule }; }
        static ev op_fune( er a, er b ) { return { a->rpn, b->rpn, op::fp_une }; }

        static ev op_ffalse( er a, er b ) { return { a->rpn, b->rpn, op::fp_false }; }
        static ev op_ftrue( er a, er b ) { return { a->rpn, b->rpn, op::fp_true }; }

        static ev op_trunc  ( er a, bw w ) { return cast( a->rpn, w, op::bv_trunc ); }
        static ev op_fptrunc( er a, bw w ) { return cast( a->rpn, w, op::fp_trunc ); }
        static ev op_sitofp ( er a, bw w ) { return cast( a->rpn, w, op::bv_stofp ); }
        static ev op_uitofp ( er a, bw w ) { return cast( a->rpn, w, op::bv_utofp ); }
        static ev op_zext   ( er a, bw w ) { return cast( a->rpn, w, op::bv_zext ); }
        static ev op_zfit   ( er a, bw w ) { return cast( a->rpn, w, op::bv_zfit ); }
        static ev op_sext   ( er a, bw w ) { return cast( a->rpn, w, op::bv_sext ); }
        static ev op_fpext  ( er a, bw w ) { return cast( a->rpn, w, op::fp_ext ); }
        static ev op_fptosi ( er a, bw w ) { return cast( a->rpn, w, op::fp_tosbv ); }
        static ev op_fptoui ( er a, bw w ) { return cast( a->rpn, w, op::fp_toubv ); }

        static std::string trace( er ) { return "expr"; }

        template< typename stream >
        friend stream& operator<<( stream &os, er e )
        {
            return os << e->rpn;
        }
    };

} // namespace __lava
