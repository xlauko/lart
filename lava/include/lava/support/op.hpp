/*
 * (c) 2020 Henrich Lauko <xlauko@mail.muni.cz>
 * (c) 2020 Petr Ročkai <code@fixp.eu>
 *
 * Permission to use, copy, modify, and distribute this software fref any
 * purpose with ref without fee is hereby granted, provided that the above
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
#include <lava/support/operations.hpp>

namespace __lava
{
    struct [[gnu::packed]] op_tag_storage
    {
        op::tag value = op::tag::unknown;

        constexpr op_tag_storage() = default;
        constexpr op_tag_storage( op::tag v ) : value( v ) {}
    };

    template< template< typename > typename storage >
    struct optag : storage< op_tag_storage > 
                , domain_mixin< optag< storage > >
    {
        using base = storage< op_tag_storage >;
        using mixin = domain_mixin< optag >;

        using bw = typename mixin::bw;
        using base::base;

        using ref = const optag &;
        using tag = op::tag;

        template< typename type > static optag lift( const type& ) { return tag::lift; }
        template< typename type > static optag any() { return tag::any; }

        template< typename size >
        static optag op_alloca( const size&, uint8_t ) { return tag::alloca; }

        static void assume( ref, bool ) {}

        static tristate to_tristate( ref ) { return mixin::fail("unsupported"); }

        /* arithmetic operations */
        static optag op_add ( ref, ref ) { return tag::add; }
        static optag op_fadd( ref, ref ) { return tag::fadd; }
        static optag op_sub ( ref, ref ) { return tag::sub; }
        static optag op_fsub( ref, ref ) { return tag::fsub; }
        static optag op_mul ( ref, ref ) { return tag::mul; }
        static optag op_fmul( ref, ref ) { return tag::fmul; }
        static optag op_udiv( ref, ref ) { return tag::udiv; }
        static optag op_sdiv( ref, ref ) { return tag::sdiv; }
        static optag op_fdiv( ref, ref ) { return tag::fdiv; }
        static optag op_urem( ref, ref ) { return tag::urem; }
        static optag op_srem( ref, ref ) { return tag::srem; }
        static optag op_frem( ref, ref ) { return tag::frem; }

        static optag op_fneg( ref ) { return tag::fneg; }

        /* bitwise operations */
        static optag op_shl ( ref, ref ) { return tag::shl; }
        static optag op_lshr( ref, ref ) { return tag::lshr; }
        static optag op_ashr( ref, ref ) { return tag::ashr; }
        static optag op_and ( ref, ref ) { return tag::op_and; }
        static optag op_or  ( ref, ref ) { return tag::op_or; }
        static optag op_xor ( ref, ref ) { return tag::op_xor; }

        /* comparison operations */
        static optag op_foeq( ref, ref ) { return tag::foeq; }
        static optag op_fogt( ref, ref ) { return tag::fogt; }
        static optag op_foge( ref, ref ) { return tag::foge; }
        static optag op_folt( ref, ref ) { return tag::folt; }
        static optag op_fole( ref, ref ) { return tag::fole; }
        static optag op_fone( ref, ref ) { return tag::fone; }
        static optag op_ford( ref, ref ) { return tag::ford; }
        static optag op_funo( ref, ref ) { return tag::funo; }
        static optag op_fueq( ref, ref ) { return tag::fueq; }
        static optag op_fugt( ref, ref ) { return tag::fugt; }
        static optag op_fuge( ref, ref ) { return tag::fuge; }
        static optag op_fult( ref, ref ) { return tag::fult; }
        static optag op_fule( ref, ref ) { return tag::fule; }
        static optag op_fune( ref, ref ) { return tag::fune; }

        static optag op_eq ( ref, ref ) { return tag::eq; }
        static optag op_ne ( ref, ref ) { return tag::ne; }
        static optag op_ugt( ref, ref ) { return tag::ugt; }
        static optag op_uge( ref, ref ) { return tag::uge; }
        static optag op_ult( ref, ref ) { return tag::ult; }
        static optag op_ule( ref, ref ) { return tag::ule; }
        static optag op_sgt( ref, ref ) { return tag::sgt; }
        static optag op_sge( ref, ref ) { return tag::sge; }
        static optag op_slt( ref, ref ) { return tag::slt; }
        static optag op_sle( ref, ref ) { return tag::sle; }

        static optag op_ffalse( ref, ref ) { return tag::ffalse; }
        static optag op_ftrue ( ref, ref ) { return tag::ftrue; }


        static optag op_fpext  ( ref, bw ) { return tag::fpext; }
        static optag op_fptosi ( ref, bw ) { return tag::fptosi; }
        static optag op_fptoui ( ref, bw ) { return tag::fptoui; }
        static optag op_fptrunc( ref, bw ) { return tag::fptrunc; }

        static optag op_inttoptr( ref, bw ) { return tag::inttoptr; }
        static optag op_ptrtoint( ref, bw ) { return tag::ptrtoint; }

        static optag op_sext  ( ref, bw ) { return tag::sext; }
        static optag op_sitofp( ref, bw ) { return tag::sitofp; }
        static optag op_trunc ( ref, bw ) { return tag::trunc; }
        static optag op_uitofp( ref, bw ) { return tag::uitofp; }
        static optag op_zext  ( ref, bw ) { return tag::zext; }
        static optag op_zfit  ( ref, bw ) { return tag::zfit; }

        static void dump( ref op ) { printf("%s\n", op::to_string(op->value).data()); }
        static std::string trace( ref op ) { return op::to_string(op->value); }
        
        template< typename stream >
        friend stream& operator<<( stream &os, ref op ) { return os << op::to_string(op->value); }

        static void bop_add ( ref, ref, ref ) { /* noop */ }
        static void bop_sub ( ref, ref, ref ) { /* noop */ }
        static void bop_mul ( ref, ref, ref ) { /* noop */ }
        static void bop_sdiv( ref, ref, ref ) { /* noop */ }
        static void bop_udiv( ref, ref, ref ) { /* noop */ }
        static void bop_srem( ref, ref, ref ) { /* noop */ }
        static void bop_urem( ref, ref, ref ) { /* noop */ }

        static void bop_fadd( ref, ref, ref ) { /* noop */ }
        static void bop_fsub( ref, ref, ref ) { /* noop */ }
        static void bop_fmul( ref, ref, ref ) { /* noop */ }
        static void bop_fdiv( ref, ref, ref ) { /* noop */ }
        static void bop_frem( ref, ref, ref ) { /* noop */ }

        static void bop_shl ( ref, ref, ref ) { /* noop */ }
        static void bop_ashr( ref, ref, ref ) { /* noop */ }
        static void bop_lshr( ref, ref, ref ) { /* noop */ }
        static void bop_and ( ref, ref, ref ) { /* noop */ }
        static void bop_or  ( ref, ref, ref ) { /* noop */ }
        static void bop_xor ( ref, ref, ref ) { /* noop */ }

        static void bop_eq ( ref, ref, ref ) { /* noop */ }
        static void bop_ne ( ref, ref, ref ) { /* noop */ }
        static void bop_ugt( ref, ref, ref ) { /* noop */ }
        static void bop_uge( ref, ref, ref ) { /* noop */ }
        static void bop_ult( ref, ref, ref ) { /* noop */ }
        static void bop_ule( ref, ref, ref ) { /* noop */ }
        static void bop_sgt( ref, ref, ref ) { /* noop */ }
        static void bop_sge( ref, ref, ref ) { /* noop */ }
        static void bop_slt( ref, ref, ref ) { /* noop */ }
        static void bop_sle( ref, ref, ref ) { /* noop */ }

        static void bop_foeq( ref, ref, ref ) { /* noop */ }
        static void bop_fogt( ref, ref, ref ) { /* noop */ }
        static void bop_foge( ref, ref, ref ) { /* noop */ }
        static void bop_folt( ref, ref, ref ) { /* noop */ }
        static void bop_fole( ref, ref, ref ) { /* noop */ }
        static void bop_ford( ref, ref, ref ) { /* noop */ }
        static void bop_funo( ref, ref, ref ) { /* noop */ }
        static void bop_fueq( ref, ref, ref ) { /* noop */ }
        static void bop_fugt( ref, ref, ref ) { /* noop */ }
        static void bop_fuge( ref, ref, ref ) { /* noop */ }
        static void bop_fult( ref, ref, ref ) { /* noop */ }
        static void bop_fule( ref, ref, ref ) { /* noop */ }

        static void bop_ffalse( ref, ref, ref ) { /* noop */ }
        static void bop_ftrue ( ref, ref, ref ) { /* noop */ }

        static void bop_trunc  ( ref, ref ) { /* noop */ }
        static void bop_fptrunc( ref, ref ) { /* noop */ }
        static void bop_sitofp ( ref, ref ) { /* noop */ }
        static void bop_uitofp ( ref, ref ) { /* noop */ }
        static void bop_zext   ( ref, ref ) { /* noop */ }
        static void bop_zfit   ( ref, ref ) { /* noop */ }
        static void bop_sext   ( ref, ref ) { /* noop */ }
        static void bop_fpext  ( ref, ref ) { /* noop */ }
        static void bop_fptosi ( ref, ref ) { /* noop */ }
        static void bop_fptoui ( ref, ref ) { /* noop */ }
    };

} // namespace __lava
