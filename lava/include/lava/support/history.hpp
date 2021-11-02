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

#include <memory>
#include <vector>

#include <runtime/vector.hpp>

#include <lava/support/base.hpp>
#include <lava/support/tristate.hpp>
#include <lava/support/operations.hpp>

namespace __lava
{
    template< typename domain >
    struct [[gnu::packed]] history_storage
    {
        constexpr history_storage(domain &&v) 
            : value( std::make_shared< domain >( std::move(v) ) )
        {}

        std::shared_ptr< domain > value;
        // TODO: shared pointers< with_shistory > to children
        std::vector< void* > children;
    };

    template< typename domain, template< typename > typename storage >
    struct with_history : storage< history_storage< domain > >
                        , domain_mixin< with_history< domain, storage > >
    {
        using base = storage< history_storage< domain > >;
        using mixin = domain_mixin< with_history >;

        using bw = typename mixin::bw;
        using base::base;

        with_history( const domain &v ) = delete; 
        
        using self = with_history;
        using sref = const with_history &;
        
        template< typename type >
        static self lift( const type &val ) { return domain::lift( val ); }

        template< typename type >
        static self any() { return domain::template any< type >(); }

        static void assume( self &a, bool expected ) { domain::assume( value(a), expected ); }

        static tristate to_tristate( sref a )
        {
            return domain::to_tristate( value(a) );
        }

        static const domain& value(sref v) { return *v->value; }
        static domain& value(self &v) { return *v->value; }

        template< typename op_t >
        static self bin( op_t op, sref a, sref b )
        {
            self r = op( value(a), value(b) );
            r->children.push_back( a.unsafe_ptr() );
            r->children.push_back( b.unsafe_ptr() );
            return r;
        }
        
        template< typename op_t >
        static self cast( op_t op, sref a, bw b )
        {
            self r = op( value(a), b );
            r->children.push_back( a.unsafe_ptr() );
            return r;
        }

        /* arithmetic operations */
        static self op_add ( sref a, sref b ) { return bin( domain::op_add, a, b ); }
        static self op_fadd( sref a, sref b ) { return bin( domain::op_fadd, a, b ); }
        static self op_sub ( sref a, sref b ) { return bin( domain::op_sub, a, b ); }
        static self op_fsub( sref a, sref b ) { return bin( domain::op_fsub, a, b ); }
        static self op_mul ( sref a, sref b ) { return bin( domain::op_mul, a, b ); }
        static self op_fmul( sref a, sref b ) { return bin( domain::op_fmul, a, b ); }
        static self op_udiv( sref a, sref b ) { return bin( domain::op_udiv, a, b ); }
        static self op_sdiv( sref a, sref b ) { return bin( domain::op_sdiv, a, b ); }
        static self op_fdiv( sref a, sref b ) { return bin( domain::op_fdiv, a, b ); }
        static self op_urem( sref a, sref b ) { return bin( domain::op_urem, a, b ); }
        static self op_srem( sref a, sref b ) { return bin( domain::op_srem, a, b ); }
        static self op_frem( sref a, sref b ) { return bin( domain::op_frem, a, b ); }

        // /* bitwise operations */
        static self op_shl ( sref a, sref b ) { return bin( domain::op_shl, a, b ); }
        static self op_lshr( sref a, sref b ) { return bin( domain::op_lshr, a, b ); }
        static self op_ashr( sref a, sref b ) { return bin( domain::op_ashr, a, b ); }
        static self op_and ( sref a, sref b ) { return bin( domain::op_and, a, b ); }
        static self op_or  ( sref a, sref b ) { return bin( domain::op_or, a, b ); }
        static self op_xor ( sref a, sref b ) { return bin( domain::op_xor, a, b ); }

        // /* comparison operations */
        static self op_foeq( sref a, sref b ) { return bin( domain::op_foeq, a, b ); }
        static self op_fogt( sref a, sref b ) { return bin( domain::op_fogt, a, b ); }
        static self op_foge( sref a, sref b ) { return bin( domain::op_foge, a, b ); }
        static self op_folt( sref a, sref b ) { return bin( domain::op_folt, a, b ); }
        static self op_fole( sref a, sref b ) { return bin( domain::op_fole, a, b ); }
        static self op_fone( sref a, sref b ) { return bin( domain::op_fone, a, b ); }
        static self op_ford( sref a, sref b ) { return bin( domain::op_ford, a, b ); }
        static self op_funo( sref a, sref b ) { return bin( domain::op_funo, a, b ); }
        static self op_fueq( sref a, sref b ) { return bin( domain::op_fueq, a, b ); }
        static self op_fugt( sref a, sref b ) { return bin( domain::op_fugt, a, b ); }
        static self op_fuge( sref a, sref b ) { return bin( domain::op_fuge, a, b ); }
        static self op_fult( sref a, sref b ) { return bin( domain::op_fult, a, b ); }
        static self op_fule( sref a, sref b ) { return bin( domain::op_fule, a, b ); }
        static self op_fune( sref a, sref b ) { return bin( domain::op_fune, a, b ); }

        static self op_eq ( sref a, sref b ) { return bin( domain::op_eq, a, b ); }
        static self op_ne ( sref a, sref b ) { return bin( domain::op_ne, a, b ); }
        static self op_ugt( sref a, sref b ) { return bin( domain::op_ugt, a, b ); }
        static self op_uge( sref a, sref b ) { return bin( domain::op_uge, a, b ); }
        static self op_ult( sref a, sref b ) { return bin( domain::op_ult, a, b ); }
        static self op_ule( sref a, sref b ) { return bin( domain::op_ule, a, b ); }
        static self op_sgt( sref a, sref b ) { return bin( domain::op_sgt, a, b ); }
        static self op_sge( sref a, sref b ) { return bin( domain::op_sge, a, b ); }
        static self op_slt( sref a, sref b ) { return bin( domain::op_slt, a, b ); }
        static self op_sle( sref a, sref b ) { return bin( domain::op_sle, a, b ); }

        static self op_ffalse( sref a, sref b ) { return bin( domain::op_ffalse, a, b ); }
        static self op_ftrue( sref a, sref b )  { return bin( domain::op_ftrue, a, b ); }

        static self op_fpext   ( sref a, bw b ) { return cast( domain::op_fpext, a, b ); }
        static self op_fptosi  ( sref a, bw b ) { return cast( domain::op_fptosi, a, b ); }
        static self op_fptoui  ( sref a, bw b ) { return cast( domain::op_fptoui, a, b ); }
        static self op_fptrunc ( sref a, bw b ) { return cast( domain::op_fptrunc, a, b ); }
        static self op_inttoptr( sref a, bw b ) { return cast( domain::op_inttoptr, a, b ); }
        static self op_ptrtoint( sref a, bw b ) { return cast( domain::op_ptrtoint, a, b ); }
        static self op_sext    ( sref a, bw b ) { return cast( domain::op_sext, a, b ); }
        static self op_sitofp  ( sref a, bw b ) { return cast( domain::op_sitofp, a, b ); }
        static self op_trunc   ( sref a, bw b ) { return cast( domain::op_trunc, a, b ); }
        static self op_uitofp  ( sref a, bw b ) { return cast( domain::op_uitofp, a, b ); }
        static self op_zext    ( sref a, bw b ) { return cast( domain::op_zext, a, b ); }
        static self op_zfit    ( sref a, bw b ) { return cast( domain::op_zfit, a, b ); }

        template< typename stream >
        friend stream& operator<<( stream &os, sref v )
        {
            return os << *v->value;
        }

        using mixin::report;

        // backward operations
        static void bop_not ( sref r, sref a ) { domain::bop_not( value(r), value(a) ); }
        static void bop_neg ( sref r, sref a ) { domain::bop_neg( value(r), value(a) ); }

        static void bop_add ( sref r, sref a, sref b ) { domain::bop_add( value(r), value(a), value(b) ); }
        static void bop_sub ( sref r, sref a, sref b ) { domain::bop_sub( value(r), value(a), value(b) ); }
        static void bop_mul ( sref r, sref a, sref b ) { domain::bop_mul( value(r), value(a), value(b) ); }
        static void bop_sdiv( sref r, sref a, sref b ) { domain::bop_sdiv( value(r), value(a), value(b) ); }
        static void bop_udiv( sref r, sref a, sref b ) { domain::bop_udiv( value(r), value(a), value(b) ); }
        static void bop_srem( sref r, sref a, sref b ) { domain::bop_srem( value(r), value(a), value(b) ); }
        static void bop_urem( sref r, sref a, sref b ) { domain::bop_urem( value(r), value(a), value(b) ); }

        static void bop_fadd( sref r, sref a, sref b ) { domain::bop_fadd( value(r), value(a), value(b) ); }
        static void bop_fsub( sref r, sref a, sref b ) { domain::bop_fsub( value(r), value(a), value(b) ); }
        static void bop_fmul( sref r, sref a, sref b ) { domain::bop_fmul( value(r), value(a), value(b) ); }
        static void bop_fdiv( sref r, sref a, sref b ) { domain::bop_fdiv( value(r), value(a), value(b) ); }
        static void bop_frem( sref r, sref a, sref b ) { domain::bop_frem( value(r), value(a), value(b) ); }

        static void bop_shl ( sref r, sref a, sref b ) { domain::bop_shl( value(r), value(a), value(b) ); }
        static void bop_ashr( sref r, sref a, sref b ) { domain::bop_ashr( value(r), value(a), value(b) ); }
        static void bop_lshr( sref r, sref a, sref b ) { domain::bop_lshr( value(r), value(a), value(b) ); }
        static void bop_and ( sref r, sref a, sref b ) { domain::bop_and( value(r), value(a), value(b) ); }
        static void bop_or  ( sref r, sref a, sref b ) { domain::bop_or( value(r), value(a), value(b) ); }
        static void bop_xor ( sref r, sref a, sref b ) { domain::bop_xor( value(r), value(a), value(b) ); }

        static void bop_eq ( sref r, sref a, sref b ) { domain::bop_eq( value(r), value(a), value(b) ); }
        static void bop_ne ( sref r, sref a, sref b ) { domain::bop_ne( value(r), value(a), value(b) ); }
        static void bop_ugt( sref r, sref a, sref b ) { domain::bop_ugt( value(r), value(a), value(b) ); }
        static void bop_uge( sref r, sref a, sref b ) { domain::bop_uge( value(r), value(a), value(b) ); }
        static void bop_ult( sref r, sref a, sref b ) { domain::bop_ult( value(r), value(a), value(b) ); }
        static void bop_ule( sref r, sref a, sref b ) { domain::bop_ule( value(r), value(a), value(b) ); }
        static void bop_sgt( sref r, sref a, sref b ) { domain::bop_sgt( value(r), value(a), value(b) ); }
        static void bop_sge( sref r, sref a, sref b ) { domain::bop_sge( value(r), value(a), value(b) ); }
        static void bop_slt( sref r, sref a, sref b ) { domain::bop_slt( value(r), value(a), value(b) ); }
        static void bop_sle( sref r, sref a, sref b ) { domain::bop_sle( value(r), value(a), value(b) ); }

        static void bop_foeq( sref r, sref a, sref b ) { domain::bop_foeq( value(r), value(a), value(b) ); }
        static void bop_fogt( sref r, sref a, sref b ) { domain::bop_fogt( value(r), value(a), value(b) ); }
        static void bop_foge( sref r, sref a, sref b ) { domain::bop_foge( value(r), value(a), value(b) ); }
        static void bop_folt( sref r, sref a, sref b ) { domain::bop_folt( value(r), value(a), value(b) ); }
        static void bop_fole( sref r, sref a, sref b ) { domain::bop_fole( value(r), value(a), value(b) ); }
        static void bop_ford( sref r, sref a, sref b ) { domain::bop_ford( value(r), value(a), value(b) ); }
        static void bop_funo( sref r, sref a, sref b ) { domain::bop_funo( value(r), value(a), value(b) ); }
        static void bop_fueq( sref r, sref a, sref b ) { domain::bop_fueq( value(r), value(a), value(b) ); }
        static void bop_fugt( sref r, sref a, sref b ) { domain::bop_fugt( value(r), value(a), value(b) ); }
        static void bop_fuge( sref r, sref a, sref b ) { domain::bop_fuge( value(r), value(a), value(b) ); }
        static void bop_fult( sref r, sref a, sref b ) { domain::bop_fult( value(r), value(a), value(b) ); }
        static void bop_fule( sref r, sref a, sref b ) { domain::bop_fule( value(r), value(a), value(b) ); }

        static void bop_ffalse( sref r, sref a, sref b ) { domain::bop_ffalse( value(r), value(a), value(b) ); }
        static void bop_ftrue ( sref r, sref a, sref b ) { domain::bop_ftrue( value(r), value(a), value(b) ); }

        static void bop_trunc  ( sref r, sref a ) { domain::bop_trunc( value(r), value(a) ); }
        static void bop_fptrunc( sref r, sref a ) { domain::bop_fptrunc( value(r), value(a) ); }
        static void bop_sitofp ( sref r, sref a ) { domain::bop_sitofp( value(r), value(a) ); }
        static void bop_uitofp ( sref r, sref a ) { domain::bop_uitofp( value(r), value(a) ); }
        static void bop_zext   ( sref r, sref a ) { domain::bop_zext( value(r), value(a) ); }
        static void bop_zfit   ( sref r, sref a ) { domain::bop_zfit( value(r), value(a) ); }
        static void bop_sext   ( sref r, sref a ) { domain::bop_sext( value(r), value(a) ); }
        static void bop_fpext  ( sref r, sref a ) { domain::bop_fpext( value(r), value(a) ); }
        static void bop_fptosi ( sref r, sref a ) { domain::bop_fptosi( value(r), value(a) ); }
        static void bop_fptoui ( sref r, sref a ) { domain::bop_fptoui( value(r), value(a) ); }

        static void bop_concat ( sref r, sref a, sref b ) { domain::bop_concat( value(r), value(a), value(b) ); }
    };

} // namespace __lava