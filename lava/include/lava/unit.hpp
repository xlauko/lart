/*
 * (c) 2020 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <lava/support/base.hpp>
#include <lava/support/tristate.hpp>

namespace __lava
{
    struct empty_storage {};

    template< template< typename > typename storage >
    struct unit : storage< empty_storage > 
                , domain_mixin< unit< storage > >
    {
        using base = storage< empty_storage >;
        using mixin = domain_mixin< unit >;

        using bw = typename mixin::bw;
        using base::base;

        using uv = unit;
        using ur = const unit &;

        template< typename type > static unit lift( const type& ) { return {}; }
        template< typename type > static unit any() { return {}; }

        template< typename size >
        static unit op_alloca( const size&, uint8_t ) { return {}; }

        static unit op_load( ur, uint8_t ) { return {}; }
        static unit op_load_at( ur, ur, uint8_t ) { return {}; }

        template< typename scalar >
        static void op_store( ur, const scalar&, uint8_t ) {}

        static void assume( ur, bool ) {}

        static tristate to_tristate( ur ) { return maybe; }

        /* arithmetic operations */
        static unit op_add ( ur, ur ) { return {}; }
        static unit op_fadd( ur, ur ) { return {}; }
        static unit op_sub ( ur, ur ) { return {}; }
        static unit op_fsub( ur, ur ) { return {}; }
        static unit op_mul ( ur, ur ) { return {}; }
        static unit op_fmul( ur, ur ) { return {}; }
        static unit op_udiv( ur, ur ) { return {}; }
        static unit op_sdiv( ur, ur ) { return {}; }
        static unit op_fdiv( ur, ur ) { return {}; }
        static unit op_urem( ur, ur ) { return {}; }
        static unit op_srem( ur, ur ) { return {}; }
        static unit op_frem( ur, ur ) { return {}; }

        static unit op_fneg( ur ) { return {}; }

        /* bitwise operations */
        static unit op_shl ( ur, ur ) { return {}; }
        static unit op_lshr( ur, ur ) { return {}; }
        static unit op_ashr( ur, ur ) { return {}; }
        static unit op_and ( ur, ur ) { return {}; }
        static unit op_or  ( ur, ur ) { return {}; }
        static unit op_xor ( ur, ur ) { return {}; }

        /* comparison operations */
        static unit op_foeq( ur, ur ) { return {}; }
        static unit op_fogt( ur, ur ) { return {}; }
        static unit op_foge( ur, ur ) { return {}; }
        static unit op_folt( ur, ur ) { return {}; }
        static unit op_fole( ur, ur ) { return {}; }
        static unit op_fone( ur, ur ) { return {}; }
        static unit op_ford( ur, ur ) { return {}; }
        static unit op_funo( ur, ur ) { return {}; }
        static unit op_fueq( ur, ur ) { return {}; }
        static unit op_fugt( ur, ur ) { return {}; }
        static unit op_fuge( ur, ur ) { return {}; }
        static unit op_fult( ur, ur ) { return {}; }
        static unit op_fule( ur, ur ) { return {}; }
        static unit op_fune( ur, ur ) { return {}; }

        static unit op_eq ( ur, ur ) { return {}; }
        static unit op_ne ( ur, ur ) { return {}; }
        static unit op_ugt( ur, ur ) { return {}; }
        static unit op_uge( ur, ur ) { return {}; }
        static unit op_ult( ur, ur ) { return {}; }
        static unit op_ule( ur, ur ) { return {}; }
        static unit op_sgt( ur, ur ) { return {}; }
        static unit op_sge( ur, ur ) { return {}; }
        static unit op_slt( ur, ur ) { return {}; }
        static unit op_sle( ur, ur ) { return {}; }

        static unit op_ffalse( ur, ur ) { return {}; }
        static unit op_ftrue( ur, ur ) { return {}; }


        static unit op_fpext( ur, bw ) { return {}; }
        static unit op_fptosi( ur, bw ) { return {}; }
        static unit op_fptoui( ur, bw ) { return {}; }
        static unit op_fptrunc( ur, bw ) { return {}; }
        static unit op_inttoptr( ur, bw ) { return {}; }
        static unit op_ptrtoint( ur, bw ) { return {}; }
        static unit op_sext( ur, bw ) { return {}; }
        static unit op_sitofp( ur, bw ) { return {}; }
        static unit op_trunc( ur, bw ) { return {}; }
        static unit op_uitofp( ur, bw ) { return {}; }
        static unit op_zext( ur, bw ) { return {}; }
        static unit op_zfit( ur, bw ) { return {}; }

        static void dump( ur ) { printf( "unit\n" ); }
        static std::string trace( ur ) { return "unit"; }
        
        template< typename stream >
        friend stream& operator<<( stream &os, ur ) { return os << "unit"; }
    };

} // namespace __lava
