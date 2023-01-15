/*
 * (c) 2022 Henrich Lauko <xlauko@mail.muni.cz>
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
    // sign domain with lattice:
    /*
    //     TOP
    //    / | \
    //   /  |  \
    // LEZ NEZ  GEZ
    //  |\ /  \/|
    //  |/ EQZ \|
    // LTZ  |  GTZ
    //   \  |  /
    //    \ | /
    //     BOT
    */

    struct [[gnu::packed]] sign_storage
    {
        using underlying_type = std::uint8_t;
        enum value_type : underlying_type {
            bot, // empty interval
            ltz, // [-∞,0)
            gtz, // (0, +∞]
            eqz, // [0,0]
            nez, // [-∞,0) ∪ (0, +∞]
            gez, // [0, +∞]
            lez, // [-∞,0]
            top, // [-∞,+∞]
            tru = gtz, // true
            fls = eqz, // false
            may = gez  // maybe
        } value;

        constexpr sign_storage() : value( top ) {}

        constexpr sign_storage( value_type v ) : value( v ) {}
    };

    template< template< typename > typename storage >
    struct sign : storage< sign_storage >
                , domain_mixin< sign< storage > >
    {
        using base = storage< sign_storage >;
        using mixin = domain_mixin< sign >;

        using bw = typename mixin::bw;
        using base::base;

        using se = sign_storage;
        using sv = sign;
        using sr = const sign &;

        using ref = domain_ref< sign >;

        __lart_inline se::value_type &value() { return this->get().value; }
        __lart_inline const se::value_type &value() const { return this->get().value; }

        __lart_inline bool is_bottom()  const { return value() == se::bot; }
        __lart_inline bool is_top()     const { return value() == se::top; }

        static sign top()    { return se::top; }
        static sign bottom() { return se::bot; }


        template< typename type > static sign lift( const type &v ) {
            if constexpr ( std::is_same_v< type, bool > )
                return v ? se::tru : se::fls;
            else if constexpr ( std::is_integral_v< type > )
                return v ? ((v < 0) ? se::ltz : se::gtz) : se::eqz;
            else
                mixin::fail( "unsupported type to lift to sign domain" );
            return se::top;
        }

        template< typename type >
        static sign any() { return top(); }

        template< typename type >
        static sign any(const variadic_list & /* args */) {
            mixin::fail("unsupported variadic any operation");
        }

        template< typename type >
        static sign any(type from, type to) {
            if (to < from)
                return bottom();
            else if (from == 0)
                return to == 0 ? se::eqz : se::gez;
            else if (to == 0)
                return se::lez;
            else if (to < 0)
                return se::ltz;
            else if (from > 0)
                return se::gtz;
            return top();
        }

        template< typename size >
        static sign op_alloca( const size&, uint8_t ) { return {}; }

        static sign op_load( sr, uint8_t ) { return {}; }
        static sign op_load_at( sr, sr, uint8_t ) { return {}; }

        template< typename scalar >
        static void op_store( sr, const scalar&, uint8_t ) {}

        static constexpr se::value_type from_bool( bool b ) {
            return b ? se::tru : se::fls;
        }

        static void assume( sign &s, bool constraint ) {
            s.value() = meet( s.value(), from_bool(constraint) );
            printf("assume %s\n", trace(s).c_str());
        }

        static tristate to_tristate( sr a ) {
            printf("to_tristate: %s\n", trace(a).c_str());
            switch (a.value()) {
                case se::bot: mixin::fail( "lowering sign bottom to tristate" ); break;
                case se::ltz: return tristate(true);
                case se::gtz: return tristate(true);
                case se::eqz: return tristate(false);
                case se::nez: return tristate(true);
                case se::gez: return maybe;
                case se::lez: return maybe;
                case se::top: return maybe;
            }

            __builtin_unreachable();
        }

        /* arithmetic operations */

        // static constexpr se::value_type template_lut[ 8 ][ 8 ] = {
        //     //               bot,     ltz,     gtz,     eqz,     nez,    gez,      lez,     top
        //     /* bot */ {  se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot },
        //     /* ltz */ {  se::bot, se::top, se::top, se::top, se::top, se::top, se::top, se::top },
        //     /* gtz */ {  se::bot, se::top, se::top, se::top, se::top, se::top, se::top, se::top },
        //     /* eqz */ {  se::bot, se::top, se::top, se::top, se::top, se::top, se::top, se::top },
        //     /* nez */ {  se::bot, se::top, se::top, se::top, se::top, se::top, se::top, se::top },
        //     /* gez */ {  se::bot, se::top, se::top, se::top, se::top, se::top, se::top, se::top },
        //     /* lez */ {  se::bot, se::top, se::top, se::top, se::top, se::top, se::top, se::top },
        //     /* top */ {  se::bot, se::top, se::top, se::top, se::top, se::top, se::top, se::top }
        // };
        static constexpr se::value_type add_lut[ 8 ][ 8 ] = {
            //               bot,     ltz,     gtz,     eqz,     nez,    gez,      lez,     top
            /* bot */ {  se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot },
            /* ltz */ {  se::bot, se::ltz, se::top, se::ltz, se::top, se::top, se::ltz, se::top },
            /* gtz */ {  se::bot, se::top, se::gtz, se::top, se::gtz, se::gtz, se::top, se::top },
            /* eqz */ {  se::bot, se::ltz, se::gtz, se::eqz, se::nez, se::gez, se::lez, se::top },
            /* nez */ {  se::bot, se::top, se::top, se::nez, se::top, se::top, se::top, se::top },
            /* gez */ {  se::bot, se::top, se::gez, se::gez, se::top, se::gez, se::top, se::top },
            /* lez */ {  se::bot, se::lez, se::top, se::lez, se::top, se::top, se::lez, se::top },
            /* top */ {  se::bot, se::top, se::top, se::top, se::top, se::top, se::top, se::top }
        };

        static constexpr se::value_type mul_lut[ 8 ][ 8 ] = {
            //               bot,     ltz,     gtz,     eqz,     nez,    gez,      lez,     top
            /* bot */ {  se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot },
            /* ltz */ {  se::bot, se::gtz, se::ltz, se::eqz, se::nez, se::lez, se::gez, se::top },
            /* gtz */ {  se::bot, se::ltz, se::gtz, se::eqz, se::nez, se::gez, se::lez, se::top },
            /* eqz */ {  se::bot, se::eqz, se::eqz, se::eqz, se::eqz, se::eqz, se::eqz, se::eqz },
            /* nez */ {  se::bot, se::nez, se::nez, se::eqz, se::nez, se::top, se::top, se::top },
            /* gez */ {  se::bot, se::lez, se::gez, se::eqz, se::top, se::gez, se::lez, se::top },
            /* lez */ {  se::bot, se::gez, se::lez, se::eqz, se::top, se::lez, se::gez, se::top },
            /* top */ {  se::bot, se::top, se::top, se::eqz, se::top, se::top, se::top, se::top }
        };

        static sign op_add ( sr a, sr b ) { return add_lut[ a->value ][ b->value ]; }
        static sign op_fadd( sr, sr ) { return {}; }
        static sign op_sub ( sr, sr ) { return {}; }
        static sign op_fsub( sr, sr ) { return {}; }
        static sign op_mul ( sr a, sr b ) { return mul_lut[ a->value ][ b->value ]; }
        static sign op_fmul( sr, sr ) { return {}; }
        static sign op_udiv( sr, sr ) { return {}; }
        static sign op_sdiv( sr, sr ) { return {}; }
        static sign op_fdiv( sr, sr ) { return {}; }
        static sign op_urem( sr, sr ) { return {}; }
        static sign op_srem( sr, sr ) { return {}; }
        static sign op_frem( sr, sr ) { return {}; }

        static sign op_fneg( sr ) { return {}; }

        /* bitwise operations */
        static sign op_shl ( sr, sr ) { return {}; }
        static sign op_lshr( sr, sr ) { return {}; }
        static sign op_ashr( sr, sr ) { return {}; }
        static sign op_and ( sr, sr ) { return {}; }
        static sign op_or  ( sr, sr ) { return {}; }
        static sign op_xor ( sr, sr ) { return {}; }

        /* comparison operations */
        static sign op_foeq( sr, sr ) { return {}; }
        static sign op_fogt( sr, sr ) { return {}; }
        static sign op_foge( sr, sr ) { return {}; }
        static sign op_folt( sr, sr ) { return {}; }
        static sign op_fole( sr, sr ) { return {}; }
        static sign op_fone( sr, sr ) { return {}; }
        static sign op_ford( sr, sr ) { return {}; }
        static sign op_funo( sr, sr ) { return {}; }
        static sign op_fueq( sr, sr ) { return {}; }
        static sign op_fugt( sr, sr ) { return {}; }
        static sign op_fuge( sr, sr ) { return {}; }
        static sign op_fult( sr, sr ) { return {}; }
        static sign op_fule( sr, sr ) { return {}; }
        static sign op_fune( sr, sr ) { return {}; }

        static constexpr se::value_type eq_lut[ 8 ][ 8 ] = {
            //               bot,     ltz,     gtz,     eqz,     nez,    gez,      lez,     top
            /* bot */ {  se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot },
            /* ltz */ {  se::bot, se::may, se::fls, se::fls, se::may, se::fls, se::may, se::may },
            /* gtz */ {  se::bot, se::fls, se::may, se::fls, se::may, se::may, se::fls, se::may },
            /* eqz */ {  se::bot, se::fls, se::fls, se::tru, se::fls, se::may, se::may, se::may },
            /* nez */ {  se::bot, se::may, se::may, se::fls, se::may, se::may, se::may, se::may },
            /* gez */ {  se::bot, se::fls, se::may, se::may, se::may, se::may, se::may, se::may },
            /* lez */ {  se::bot, se::may, se::fls, se::may, se::may, se::may, se::may, se::may },
            /* top */ {  se::bot, se::may, se::may, se::may, se::may, se::may, se::may, se::may }
        };

        static constexpr se::value_type ne_lut[ 8 ][ 8 ] = {
            //               bot,     ltz,     gtz,     eqz,     nez,    gez,      lez,     top
            /* bot */ {  se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot },
            /* ltz */ {  se::bot, se::may, se::tru, se::tru, se::may, se::tru, se::may, se::may },
            /* gtz */ {  se::bot, se::tru, se::may, se::tru, se::may, se::may, se::tru, se::may },
            /* eqz */ {  se::bot, se::tru, se::tru, se::fls, se::tru, se::may, se::may, se::may },
            /* nez */ {  se::bot, se::may, se::may, se::tru, se::may, se::may, se::may, se::may },
            /* gez */ {  se::bot, se::tru, se::may, se::may, se::may, se::may, se::may, se::may },
            /* lez */ {  se::bot, se::may, se::tru, se::may, se::may, se::may, se::may, se::may },
            /* top */ {  se::bot, se::may, se::may, se::may, se::may, se::may, se::may, se::may }
        };

        static constexpr se::value_type slt_lut[ 8 ][ 8 ] = {
            //               bot,     ltz,     gtz,     eqz,     nez,    gez,      lez,     top
            /* bot */ {  se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot },
            /* ltz */ {  se::bot, se::may, se::tru, se::tru, se::may, se::tru, se::may, se::may },
            /* gtz */ {  se::bot, se::fls, se::may, se::fls, se::may, se::may, se::fls, se::may },
            /* eqz */ {  se::bot, se::fls, se::tru, se::fls, se::may, se::may, se::may, se::may },
            /* nez */ {  se::bot, se::may, se::may, se::may, se::may, se::may, se::may, se::may },
            /* gez */ {  se::bot, se::fls, se::may, se::may, se::may, se::may, se::may, se::may },
            /* lez */ {  se::bot, se::may, se::tru, se::may, se::may, se::may, se::may, se::may },
            /* top */ {  se::bot, se::may, se::may, se::may, se::may, se::may, se::may, se::may }
        };

        static sign op_eq ( sr a, sr b ) { return eq_lut[ a->value ][ b->value ]; }
        static sign op_ne ( sr a, sr b ) { return ne_lut[ a->value ][ b->value ]; }
        static sign op_ugt( sr, sr ) { return {}; }
        static sign op_uge( sr, sr ) { return {}; }
        static sign op_ult( sr, sr ) { return {}; }
        static sign op_ule( sr, sr ) { return {}; }
        static sign op_sgt( sr a, sr b ) { return slt_lut[ b->value ][ a->value ]; }
        static sign op_sge( sr, sr ) { return {}; }
        static sign op_slt( sr a, sr b ) { return slt_lut[ a->value ][ b->value ]; }
        static sign op_sle( sr, sr ) { return {}; }

        static sign op_ffalse( sr, sr ) { return se::eqz; }
        static sign op_ftrue( sr, sr ) { return {}; }

        static sign op_fpext( sr, bw ) { return {}; }
        static sign op_fptosi( sr, bw ) { return {}; }
        static sign op_fptoui( sr, bw ) { return {}; }
        static sign op_fptrunc( sr, bw ) { return {}; }
        static sign op_inttoptr( sr, bw ) { return {}; }
        static sign op_ptrtoint( sr, bw ) { return {}; }
        static sign op_sext( sr, bw ) { return {}; }
        static sign op_sitofp( sr, bw ) { return {}; }
        static sign op_trunc( sr, bw ) { return {}; }
        static sign op_uitofp( sr, bw ) { return {}; }
        static sign op_zext( sr, bw ) { return {}; }
        static sign op_zfit( sr, bw ) { return {}; }

        static std::string trace( sr v ) {
            switch ( v->value ) {
                case se::bot:  return "⊥";
                case se::ltz:  return "[-∞,0)";
                case se::gtz:  return "(0, +∞]";
                case se::eqz:  return "[0, 0]";
                case se::nez:  return "[-∞,0) ∪ (0, +∞]";
                case se::gez:  return "[0, +∞]";
                case se::lez:  return "[-∞,0]";
                case se::top:  return "⊤";
            }

            __builtin_unreachable();
        }

        template< typename stream >
        friend stream& operator<<( stream &os, sr v ) { return os << trace(v); }

        /* set operations */

        static constexpr se::value_type meet_lut[ 8 ][ 8 ] = {
            //               bot,     ltz,     gtz,     eqz,     nez,    gez,      lez,     top
            /* bot */ {  se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot },
            /* ltz */ {  se::bot, se::ltz, se::bot, se::bot, se::ltz, se::bot, se::ltz, se::ltz },
            /* gtz */ {  se::bot, se::bot, se::gtz, se::bot, se::gtz, se::gtz, se::bot, se::gtz },
            /* eqz */ {  se::bot, se::bot, se::bot, se::eqz, se::bot, se::eqz, se::eqz, se::eqz },
            /* nez */ {  se::bot, se::ltz, se::gtz, se::bot, se::nez, se::gtz, se::ltz, se::nez },
            /* gez */ {  se::bot, se::bot, se::gtz, se::eqz, se::gtz, se::gez, se::eqz, se::gez },
            /* lez */ {  se::bot, se::ltz, se::bot, se::eqz, se::ltz, se::eqz, se::lez, se::lez },
            /* top */ {  se::bot, se::ltz, se::gtz, se::eqz, se::nez, se::gez, se::lez, se::top }
        };

        static constexpr se::value_type meet( se::value_type a, se::value_type b ) {
            return meet_lut[ a ][ b ];
        }

        static constexpr sign meet( sr a, sr b ) { return meet( a->value, b->value ); }

        static constexpr se::value_type join_lut[ 8 ][ 8 ] = {
            //               bot,     ltz,     gtz,     eqz,     nez,    gez,      lez,     top
            /* bot */ {  se::bot, se::ltz, se::gtz, se::eqz, se::nez, se::gez, se::lez, se::top },
            /* ltz */ {  se::ltz, se::ltz, se::nez, se::lez, se::nez, se::top, se::lez, se::top },
            /* gtz */ {  se::gtz, se::nez, se::gtz, se::gez, se::nez, se::gez, se::top, se::top },
            /* eqz */ {  se::eqz, se::lez, se::gez, se::eqz, se::top, se::gez, se::lez, se::top },
            /* nez */ {  se::nez, se::nez, se::nez, se::top, se::nez, se::top, se::top, se::top },
            /* gez */ {  se::gez, se::top, se::gez, se::gez, se::top, se::gez, se::top, se::top },
            /* lez */ {  se::lez, se::lez, se::top, se::lez, se::top, se::top, se::lez, se::top },
            /* top */ {  se::top, se::top, se::top, se::top, se::top, se::top, se::top, se::top }
        };

        static constexpr se::value_type join( se::value_type a, se::value_type b ) {
            return join_lut[ a ][ b ];
        }

        static constexpr sign join( sr a, sr b ) { return join( a->value, b->value ); }

        static constexpr se::value_type complement( se::value_type a ) {
            switch(a) {
                case se::bot: return se::top;
                case se::ltz: return se::gez;
                case se::gtz: return se::lez;
                case se::eqz: return se::nez;
                case se::nez: return se::eqz;
                case se::gez: return se::ltz;
                case se::lez: return se::gtz;
                case se::top: return se::bot;
            }
        }

        static constexpr sign complement( sr a ) { return complement( a->value ); }

        static constexpr se::value_type setminus_lut[ 8 ][ 8 ] = {
            //               bot,     ltz,     gtz,     eqz,     nez,    gez,      lez,     top
            /* bot */ {  se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot, se::bot },
            /* ltz */ {  se::ltz, se::bot, se::ltz, se::ltz, se::bot, se::ltz, se::bot, se::bot },
            /* gtz */ {  se::gtz, se::gtz, se::bot, se::gtz, se::bot, se::bot, se::gtz, se::bot },
            /* eqz */ {  se::eqz, se::eqz, se::eqz, se::bot, se::bot, se::bot, se::bot, se::bot },
            /* nez */ {  se::nez, se::gtz, se::ltz, se::nez, se::bot, se::ltz, se::gtz, se::bot },
            /* gez */ {  se::gez, se::gez, se::eqz, se::gtz, se::eqz, se::bot, se::gtz, se::bot },
            /* lez */ {  se::lez, se::eqz, se::lez, se::ltz, se::eqz, se::ltz, se::bot, se::bot },
            /* top */ {  se::top, se::gez, se::lez, se::nez, se::eqz, se::ltz, se::gtz, se::bot }
        };

        static constexpr se::value_type setminus( se::value_type a, se::value_type b ) {
            return setminus_lut[ a ][ b ];
        }

        static constexpr sign setminus( sr a, sr b ) { return setminus( a->value, b->value ); }

        /* backward operations */
        static void bop_sgt_impl( sr r, sign &a, sign &b ) {
            if ( r.value() == se::tru ) { // a > b
                a = setminus(a, b);
                // TODO
            } else {
                // TODO
            }
        }

        static void bop_ne_impl( sr r, sign &a, sign &b ) {
            if ( r.value() == se::tru ) { // a > b
                a = setminus(a, b);
                // TODO
            } else {
                // TODO
            }
        }

        // static void bop_add ( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_sub ( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_mul ( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_sdiv( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_udiv( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_srem( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_urem( sr r, sr a, sr b ) { /* noop */ }

        // static void bop_fadd( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_fsub( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_fmul( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_fdiv( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_frem( sr r, sr a, sr b ) { /* noop */ }

        // static void bop_shl ( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_ashr( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_lshr( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_and ( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_or  ( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_xor ( sr r, sr a, sr b ) { /* noop */ }

        // static void bop_eq ( sr r, sr a, sr b ) { /* noop */ }
        static void bop_ne ( sr r, ref a, ref b ) { bop_ne_impl( r, a, b ); }
        // static void bop_ugt( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_uge( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_ult( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_ule( sr r, sr a, sr b ) { /* noop */ }
        static void bop_sgt( sr r, ref a, ref b ) { bop_sgt_impl( r, a, b ); }
        // static void bop_sge( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_slt( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_sle( sr r, sr a, sr b ) { /* noop */ }

        // static void bop_foeq( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_fogt( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_foge( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_folt( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_fole( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_ford( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_funo( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_fueq( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_fugt( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_fuge( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_fult( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_fule( sr r, sr a, sr b ) { /* noop */ }

        // static void bop_ffalse( sr r, sr a, sr b ) { /* noop */ }
        // static void bop_ftrue ( sr r, sr a, sr b ) { /* noop */ }

        // static void bop_trunc  ( sr r, sr a ) { /* noop */ }
        // static void bop_fptrunc( sr r, sr a ) { /* noop */ }
        // static void bop_sitofp ( sr r, sr a ) { /* noop */ }
        // static void bop_uitofp ( sr r, sr a ) { /* noop */ }
        // static void bop_zext   ( sr r, sr a ) { /* noop */ }
        // static void bop_zfit   ( sr r, sr a ) { /* noop */ }
        // static void bop_sext   ( sr r, sr a ) { /* noop */ }
        // static void bop_fpext  ( sr r, sr a ) { /* noop */ }
        // static void bop_fptosi ( sr r, sr a ) { /* noop */ }
        // static void bop_fptoui ( sr r, sr a ) { /* noop */ }
    };

} // namespace __lava
