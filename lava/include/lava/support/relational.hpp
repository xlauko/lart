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

        static bool memoize( sref v, void* twin, unsigned int line ) {
            //printf("in relational\n");
            return base::memoize( v, twin, line );
        }
        /*
        template< typename... Args >
        static void memoize_var( unsigned int line, Args&&... args ) {
            base::memoize_var( line, std::forward<Args>( args )... );
        }
        */
        template < typename T, typename R >
        static void memoize_var( unsigned int line, T& twins, R& refs ) {
            printf("in relational\n");
            base::memoize_var( line, twins, refs );
        }

        //static void dump( sref v ) {
        //    base::dump( v );
        //}

        static constexpr auto wrap = __lava::op::wrap;

        static void backward_propagate( sref a )
        {
            using ref = domain_ref< base >;

            auto bin = [&] (const auto &bop ) {
                wrap( bop )( a.underlying(), ref( a.arg(0) ), ref( a.arg(1) ) );
            };

            auto un = [&] (const auto &bop ) {
                wrap( bop )( a.underlying(), ref( a.arg(0) ) );
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

                case op::tag::add: bin( op::backward::add ); break;
                case op::tag::sub: bin( op::backward::sub ); break;
                case op::tag::mul: bin( op::backward::mul ); break;
                case op::tag::sdiv: bin( op::backward::sdiv ); break;
                case op::tag::udiv: bin( op::backward::udiv ); break;
                case op::tag::srem: bin( op::backward::srem ); break;
                case op::tag::urem: bin( op::backward::urem ); break;

                case op::tag::fadd: bin( op::backward::fadd ); break;
                case op::tag::fsub: bin( op::backward::fsub ); break;
                case op::tag::fmul: bin( op::backward::fmul ); break;
                case op::tag::fdiv: bin( op::backward::fdiv ); break;
                case op::tag::frem: bin( op::backward::frem ); break;

                case op::tag::fneg: mixin::fail("unsupported fneg bop");

                case op::tag::op_and: bin( op::backward::op_and ); break;
                case op::tag::op_or:  bin( op::backward::op_or ); break;
                case op::tag::op_xor: bin( op::backward::op_xor ); break;

                case op::tag::zfit:  un( op::backward::zfit ); break;
                case op::tag::zext:  un( op::backward::zext ); break;
                case op::tag::sext:  un( op::backward::sext ); break;
                case op::tag::trunc: un( op::backward::trunc ); break;

                case op::tag::fptrunc: un( op::backward::fptrunc ); break;
                case op::tag::sitofp:  un( op::backward::sitofp ); break;
                case op::tag::uitofp:  un( op::backward::uitofp ); break;
                case op::tag::fptosi:  un( op::backward::fptosi ); break;
                case op::tag::fptoui:  un( op::backward::fptoui ); break;
                case op::tag::fpext:   un( op::backward::fpext ); break;

                case op::tag::inttoptr: mixin::fail("unsupported inttoptr bop");
                case op::tag::ptrtoint: mixin::fail("unsupported ptrtoint bop");

                case op::tag::ne: bin( op::backward::ne ); break;
                case op::tag::eq: bin( op::backward::eq ); break;

                case op::tag::slt: bin( op::backward::slt ); break;
                case op::tag::sgt: bin( op::backward::sgt ); break;
                case op::tag::sle: bin( op::backward::sle ); break;
                case op::tag::sge: bin( op::backward::sge ); break;

                case op::tag::ult: bin( op::backward::ult ); break;
                case op::tag::ugt: bin( op::backward::ugt ); break;
                case op::tag::ule: bin( op::backward::ule ); break;
                case op::tag::uge: bin( op::backward::uge ); break;

                case op::tag::foeq: bin( op::backward::foeq ); break;
                case op::tag::fogt: bin( op::backward::fogt ); break;
                case op::tag::foge: bin( op::backward::foge ); break;
                case op::tag::folt: bin( op::backward::folt ); break;
                case op::tag::fole: bin( op::backward::fole ); break;
                case op::tag::fone: bin( op::backward::fone ); break;
                case op::tag::ford: bin( op::backward::ford ); break;
                case op::tag::funo: bin( op::backward::funo ); break;
                case op::tag::fueq: bin( op::backward::fueq ); break;
                case op::tag::fugt: bin( op::backward::fugt ); break;
                case op::tag::fuge: bin( op::backward::fuge ); break;
                case op::tag::fult: bin( op::backward::fult ); break;
                case op::tag::fule: bin( op::backward::fule ); break;
                case op::tag::fune: bin( op::backward::fune ); break;

                case op::tag::ffalse: bin( op::backward::ffalse ); break;
                case op::tag::ftrue:  bin( op::backward::ftrue ); break;

                case op::tag::shl:  bin( op::backward::shl ); break;
                case op::tag::lshr: bin( op::backward::lshr ); break;
                case op::tag::ashr: bin( op::backward::ashr ); break;

                case op::tag::concat: mixin::fail("unsupported concat bop");
                case op::tag::extract: mixin::fail("unsupported extract bop");

                case op::tag::malloc: mixin::fail("unsupported malloc bop");
                case op::tag::realloc: mixin::fail("unsupported realloc bop");

                case op::tag::dealloca: mixin::fail("unsupported dealloca bop");
                case op::tag::free: mixin::fail("unsupported free bop");
            }

            for (const auto &ch : a.underlying()->children) {
                using rel_ref = domain_ref< self >;
                backward_propagate( rel_ref(ch) );
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