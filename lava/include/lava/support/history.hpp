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
        std::vector< std::shared_ptr< domain > > children;
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
        with_history( domain &&v ) : base(std::move(v)) {}
        
        using self = with_history;
        using sref = const with_history &;
        
        template< typename type >
        static self lift( const type &val ) { return domain::lift( val ); }

        template< typename type >
        static self any() { return domain::template any< type >(); }

        static void assume( self &a, bool expected ) { domain::assume( *a->value, expected ); }

        static tristate to_tristate( sref a )
        {
            return domain::to_tristate( *a->value );
        }

        template< typename op_t >
        static self bin( op_t op, sref a, sref b )
        {
            self r = op( *a->value, *b->value );
            r->children.push_back( a->value );
            r->children.push_back( b->value );
            return r;
        }
        
        template< typename op_t >
        static self cast( op_t op, sref a, bw b )
        {
            self r = op( *a->value, b );
            r->children.push_back( a->value );
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

        static void dump( sref v )
        {
            //apply( domain::dump, a, b );
        }

        // static std::string trace( sref v ) { 
        //     return apply( domain::trace, a, b ); }

        template< typename stream >
        friend stream& operator<<( stream &os, sref v )
        {
            os << *v->value;
            for (const auto &ch : v->children)
                os << " " << *ch;
            return os;
        }
    };

} // namespace __lava