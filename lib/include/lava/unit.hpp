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
#include <lava/constant.hpp>

namespace __lava
{
    struct unit : tagged_array, domain_mixin< unit > {
 using index_dom = unit;

        using tagged_array::tagged_array;
        using base = domain_mixin< unit >;

        template< typename type > static unit lift( type ) { return {}; }
        template< typename type > static unit any() { return {}; }

        static constant lower( unit )
        {
            base::fail();
            __builtin_unreachable();
        }

        static unit lift_any() { return {}; }
        static unit lift_any_ptr() { return {}; }

        template< typename size >
        static unit op_alloca( const size&, uint8_t ) { return {}; }

        static unit op_load( unit, uint8_t ) { return {}; }
        static unit op_load_at( unit, unit, uint8_t ) { return {}; }

        template< typename scalar >
        static unit op_store( unit, scalar, uint8_t ) { return {}; }

        static unit assume( unit, bool ) { return {}; }

        static tristate to_tristate( unit ) { return maybe; }

        /* arithmetic operations */
        static unit op_add ( unit, unit ) { return {}; }
        static unit op_fadd( unit, unit ) { return {}; }
        static unit op_sub ( unit, unit ) { return {}; }
        static unit op_fsub( unit, unit ) { return {}; }
        static unit op_mul ( unit, unit ) { return {}; }
        static unit op_fmul( unit, unit ) { return {}; }
        static unit op_udiv( unit, unit ) { return {}; }
        static unit op_sdiv( unit, unit ) { return {}; }
        static unit op_fdiv( unit, unit ) { return {}; }
        static unit op_urem( unit, unit ) { return {}; }
        static unit op_srem( unit, unit ) { return {}; }
        static unit op_frem( unit, unit ) { return {}; }

        static unit op_fneg( unit ) { return {}; }

        /* bitwise operations */
        static unit op_shl ( unit, unit ) { return {}; }
        static unit op_lshr( unit, unit ) { return {}; }
        static unit op_ashr( unit, unit ) { return {}; }
        static unit op_and ( unit, unit ) { return {}; }
        static unit op_or  ( unit, unit ) { return {}; }
        static unit op_xor ( unit, unit ) { return {}; }

        using bw = uint8_t;

        /* comparison operations */
        static unit op_foeq( unit, unit ) { return {}; }
        static unit op_fogt( unit, unit ) { return {}; }
        static unit op_foge( unit, unit ) { return {}; }
        static unit op_folt( unit, unit ) { return {}; }
        static unit op_fole( unit, unit ) { return {}; }
        static unit op_fone( unit, unit ) { return {}; }
        static unit op_ford( unit, unit ) { return {}; }
        static unit op_funo( unit, unit ) { return {}; }
        static unit op_fueq( unit, unit ) { return {}; }
        static unit op_fugt( unit, unit ) { return {}; }
        static unit op_fuge( unit, unit ) { return {}; }
        static unit op_fult( unit, unit ) { return {}; }
        static unit op_fule( unit, unit ) { return {}; }
        static unit op_fune( unit, unit ) { return {}; }

        static unit op_eq ( unit, unit ) { return {}; }
        static unit op_ne ( unit, unit ) { return {}; }
        static unit op_ugt( unit, unit ) { return {}; }
        static unit op_uge( unit, unit ) { return {}; }
        static unit op_ult( unit, unit ) { return {}; }
        static unit op_ule( unit, unit ) { return {}; }
        static unit op_sgt( unit, unit ) { return {}; }
        static unit op_sge( unit, unit ) { return {}; }
        static unit op_slt( unit, unit ) { return {}; }
        static unit op_sle( unit, unit ) { return {}; }

        static unit op_ffalse( unit, unit ) { return {}; }
        static unit op_ftrue( unit, unit ) { return {}; }


        static unit op_fpext( unit, bw ) { return {}; }
        static unit op_fptosi( unit, bw ) { return {}; }
        static unit op_fptoui( unit, bw ) { return {}; }
        static unit op_fptrunc( unit, bw ) { return {}; }
        static unit op_inttoptr( unit, bw ) { return {}; }
        static unit op_ptrtoint( unit, bw ) { return {}; }
        static unit op_sext( unit, bw ) { return {}; }
        static unit op_sitofp( unit, bw ) { return {}; }
        static unit op_trunc( unit, bw ) { return {}; }
        static unit op_uitofp( unit, bw ) { return {}; }
        static unit op_zext( unit, bw ) { return {}; }
        static unit op_zfit( unit, bw ) { return {}; }

        static const char* trace( unit ) { return "unit"; }
    };

} // namespace __lava
