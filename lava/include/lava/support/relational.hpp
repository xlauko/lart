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

#include <lava/support/base.hpp>
#include <lava/support/tristate.hpp>
#include <lava/support/operations.hpp>

#include <lava/support/op.hpp>
#include <lava/support/history.hpp>
#include <lava/support/product.hpp>

namespace __lava
{
    using config = __lava::product_config< __lava::lower_first, __lava::to_tristate_first >;

    template< typename domain, template< typename > typename storage >
    using tagged = product< domain, optag< storage >, storage, config >;

    template< typename domain, template< typename > typename storage >
    struct relational : with_history< tagged< domain, storage >, storage >
    {
        using base  = with_history< tagged< domain, storage >, storage >;
        using mixin = domain_mixin< relational >;

        using optag = typename tagged< domain, storage >::right_type;

        using bw = typename mixin::bw;

        using base::base;
        
        relational( const relational &v ) = delete;
        relational( relational&& ) = default;

        relational( base &&v ) : base(std::move(v)) {}

        using self = relational;
        using sref = const relational &;

        const auto &underlying() const
        {
            return static_cast<const base &>( *this );
        }

        constexpr op::tag tag() const
        {
            const auto &tagged = underlying()->value;
            return tagged->right()->value;
        }
        
        constexpr void* arg(unsigned idx) const
        {
            return underlying()->children[idx];
        }

        template< typename type >
        static self lift( const type &val ) { return base::lift( val ); }

        template< typename type >
        static self any() { return base::template any< type >(); }

        static void assume( self &a, bool expected )
        {
            base::assume( a, expected );
            backward_propagate( a );
        }

        static constexpr auto wrap = __lava::op::wrap;

        static void backward_propagate( self &a )
        {
            using ref = domain_ref< base >;

            auto bin = [&] (const auto &bop ) {
                wrap( bop )( a.underlying(), ref( a.arg(1) ), ref( a.arg(2) ) );
            };

            auto un = [&] (const auto &bop ) {
                wrap( bop )( a.underlying(), ref( a.arg(1) ) );
            };

            switch (a.tag())
            {
                case op::tag::unknown: mixin::fail("unknown op tag");

                case op::tag::any:   return;
                case op::tag::lift:  return;
                case op::tag::lower: return;

                case op::tag::join: mixin::fail("unsupported join bop");
                case op::tag::meet: mixin::fail("unsupported meet bop");
                
                case op::tag::alloca: mixin::fail("unsupported alloca bop");
                case op::tag::store:  mixin::fail("unsupported store bop");
                case op::tag::load:   mixin::fail("unsupported load bop");

                case op::tag::add: return bin( op::backward::add );
                case op::tag::sub: return bin( op::backward::sub );
                case op::tag::mul: return bin( op::backward::mul );
                case op::tag::sdiv: return bin( op::backward::sdiv );
                case op::tag::udiv: return bin( op::backward::udiv );
                case op::tag::srem: return bin( op::backward::srem );
                case op::tag::urem: return bin( op::backward::urem );

                case op::tag::fadd: return bin( op::backward::fadd );
                case op::tag::fsub: return bin( op::backward::fsub );
                case op::tag::fmul: return bin( op::backward::fmul );
                case op::tag::fdiv: return bin( op::backward::fdiv );
                case op::tag::frem: return bin( op::backward::frem );

                case op::tag::fneg: mixin::fail("unsupported fneg bop");
                
                case op::tag::op_and: return bin( op::backward::op_and );
                case op::tag::op_or: return bin( op::backward::op_or );
                case op::tag::op_xor: return bin( op::backward::op_xor );

                case op::tag::zfit: return un( op::backward::zfit );
                case op::tag::zext: return un( op::backward::zext ); 
                case op::tag::sext: return un( op::backward::sext ); 
                case op::tag::trunc: return un( op::backward::trunc );

                case op::tag::fptrunc: return un( op::backward::fptrunc );
                case op::tag::sitofp: return un( op::backward::sitofp );
                case op::tag::uitofp: return un( op::backward::uitofp );
                case op::tag::fptosi: return un( op::backward::fptosi );
                case op::tag::fptoui: return un( op::backward::fptoui );
                case op::tag::fpext:  return un( op::backward::fpext );

                case op::tag::inttoptr: mixin::fail("unsupported inttoptr bop");
                case op::tag::ptrtoint: mixin::fail("unsupported ptrtoint bop");

                case op::tag::ne: return bin( op::backward::ne );
                case op::tag::eq: return bin( op::backward::eq );

                case op::tag::slt: return bin( op::backward::slt );
                case op::tag::sgt: return bin( op::backward::sgt );
                case op::tag::sle: return bin( op::backward::sle );
                case op::tag::sge: return bin( op::backward::sge );

                case op::tag::ult: return bin( op::backward::ult );
                case op::tag::ugt: return bin( op::backward::ugt );
                case op::tag::ule: return bin( op::backward::ule );
                case op::tag::uge: return bin( op::backward::uge );

                case op::tag::foeq: return bin( op::backward::foeq );
                case op::tag::fogt: return bin( op::backward::fogt );
                case op::tag::foge: return bin( op::backward::foge );
                case op::tag::folt: return bin( op::backward::folt );
                case op::tag::fole: return bin( op::backward::fole );
                case op::tag::fone: return bin( op::backward::fone );
                case op::tag::ford: return bin( op::backward::ford ); 
                case op::tag::funo: return bin( op::backward::funo );
                case op::tag::fueq: return bin( op::backward::fueq );
                case op::tag::fugt: return bin( op::backward::fugt );
                case op::tag::fuge: return bin( op::backward::fuge );
                case op::tag::fult: return bin( op::backward::fult );
                case op::tag::fule: return bin( op::backward::fule );
                case op::tag::fune: return bin( op::backward::fune );

                case op::tag::ffalse: return bin( op::backward::ffalse );
                case op::tag::ftrue:  return bin( op::backward::ftrue );

                case op::tag::shl:  return bin( op::backward::shl );
                case op::tag::lshr: return bin( op::backward::lshr );
                case op::tag::ashr: return bin( op::backward::ashr );

                case op::tag::concat: mixin::fail("unsupported concat bop");
                case op::tag::extract: mixin::fail("unsupported extract bop");

                case op::tag::malloc: mixin::fail("unsupported malloc bop");
                case op::tag::realloc: mixin::fail("unsupported realloc bop");
                
                case op::tag::dealloca: mixin::fail("unsupported dealloca bop");
                case op::tag::free: mixin::fail("unsupported free bop");
            }
        }

        static tristate to_tristate( sref a )
        {
            return base::to_tristate( a );
        }

        /* arithmetic operations */
        static self op_add ( sref a, sref b ) { return base::op_add( a, b ); }
        static self op_fadd( sref a, sref b ) { return base::op_fadd( a, b ); }
        static self op_sub ( sref a, sref b ) { return base::op_sub( a, b ); }
        static self op_fsub( sref a, sref b ) { return base::op_fsub( a, b ); }
        static self op_mul ( sref a, sref b ) { return base::op_mul( a, b ); }
        static self op_fmul( sref a, sref b ) { return base::op_fmul( a, b ); }
        static self op_udiv( sref a, sref b ) { return base::op_udiv( a, b ); }
        static self op_sdiv( sref a, sref b ) { return base::op_sdiv( a, b ); }
        static self op_fdiv( sref a, sref b ) { return base::op_fdiv( a, b ); }
        static self op_urem( sref a, sref b ) { return base::op_urem( a, b ); }
        static self op_srem( sref a, sref b ) { return base::op_srem( a, b ); }
        static self op_frem( sref a, sref b ) { return base::op_frem( a, b ); }

        // /* bitwise operations */
        static self op_shl ( sref a, sref b ) { return base::op_shl( a, b ); }
        static self op_lshr( sref a, sref b ) { return base::op_lshr( a, b ); }
        static self op_ashr( sref a, sref b ) { return base::op_ashr( a, b ); }
        static self op_and ( sref a, sref b ) { return base::op_and( a, b ); }
        static self op_or  ( sref a, sref b ) { return base::op_or( a, b ); }
        static self op_xor ( sref a, sref b ) { return base::op_xor( a, b ); }

        // /* comparison operations */
        static self op_foeq( sref a, sref b ) { return base::op_foeq( a, b ); }
        static self op_fogt( sref a, sref b ) { return base::op_fogt( a, b ); }
        static self op_foge( sref a, sref b ) { return base::op_foge( a, b ); }
        static self op_folt( sref a, sref b ) { return base::op_folt( a, b ); }
        static self op_fole( sref a, sref b ) { return base::op_fole( a, b ); }
        static self op_fone( sref a, sref b ) { return base::op_fone( a, b ); }
        static self op_ford( sref a, sref b ) { return base::op_ford( a, b ); }
        static self op_funo( sref a, sref b ) { return base::op_funo( a, b ); }
        static self op_fueq( sref a, sref b ) { return base::op_fueq( a, b ); }
        static self op_fugt( sref a, sref b ) { return base::op_fugt( a, b ); }
        static self op_fuge( sref a, sref b ) { return base::op_fuge( a, b ); }
        static self op_fult( sref a, sref b ) { return base::op_fult( a, b ); }
        static self op_fule( sref a, sref b ) { return base::op_fule( a, b ); }
        static self op_fune( sref a, sref b ) { return base::op_fune( a, b ); }

        static self op_eq ( sref a, sref b ) { return base::op_eq( a, b ); }
        static self op_ne ( sref a, sref b ) { return base::op_ne( a, b ); }
        static self op_ugt( sref a, sref b ) { return base::op_ugt( a, b ); }
        static self op_uge( sref a, sref b ) { return base::op_uge( a, b ); }
        static self op_ult( sref a, sref b ) { return base::op_ult( a, b ); }
        static self op_ule( sref a, sref b ) { return base::op_ule( a, b ); }
        static self op_sgt( sref a, sref b ) { return base::op_sgt( a, b ); }
        static self op_sge( sref a, sref b ) { return base::op_sge( a, b ); }
        static self op_slt( sref a, sref b ) { return base::op_slt( a, b ); }
        static self op_sle( sref a, sref b ) { return base::op_sle( a, b ); }

        static self op_ffalse( sref a, sref b ) { return base::op_ffalse( a, b ); }
        static self op_ftrue( sref a, sref b )  { return base::op_ftrue( a, b ); }

        static self op_fpext   ( sref a, bw b ) { return base::op_fpext( a, b ); }
        static self op_fptosi  ( sref a, bw b ) { return base::op_fptosi( a, b ); }
        static self op_fptoui  ( sref a, bw b ) { return base::op_fptoui( a, b ); }
        static self op_fptrunc ( sref a, bw b ) { return base::op_fptrunc( a, b ); }
        static self op_inttoptr( sref a, bw b ) { return base::op_inttoptr( a, b ); }
        static self op_ptrtoint( sref a, bw b ) { return base::op_ptrtoint( a, b ); }
        static self op_sext    ( sref a, bw b ) { return base::op_sext( a, b ); }
        static self op_sitofp  ( sref a, bw b ) { return base::op_sitofp( a, b ); }
        static self op_trunc   ( sref a, bw b ) { return base::op_trunc( a, b ); }
        static self op_uitofp  ( sref a, bw b ) { return base::op_uitofp( a, b ); }
        static self op_zext    ( sref a, bw b ) { return base::op_zext( a, b ); }
        static self op_zfit    ( sref a, bw b ) { return base::op_zfit( a, b ); }
        
        template< typename stream >
        friend stream& operator<<( stream &os, sref v )
        {
            return os << static_cast<const base &>(v);
        }
    };

} // namespace __lava