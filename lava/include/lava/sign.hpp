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
            top  // [-∞,+∞]
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

        __lart_inline se::value_type value() const { return this->get().value; }

        __lart_inline bool is_bottom()  const { return value() == se::bot; }
        __lart_inline bool is_top()     const { return value() == se::top; }

        static sign top()    { return se::top; }
        static sign bottom() { return se::bot; }

        template< typename type > static sign lift( const type& ) { return {}; }
        template< typename type >
        static sign any() { return top(); }

        template< typename type >
        static sign any(type /* from */, type /* to */) {
            mixin::fail("unsupported range any");
        }

        template< typename size >
        static sign op_alloca( const size&, uint8_t ) { return {}; }

        static sign op_load( sr, uint8_t ) { return {}; }
        static sign op_load_at( sr, sr, uint8_t ) { return {}; }

        template< typename scalar >
        static void op_store( sr, const scalar&, uint8_t ) {}

        static void assume( sr, bool ) {}

        static tristate to_tristate( sr ) { return maybe; }

        /* arithmetic operations */

        // static constexpr se::value_type add_lut[ 8 ][ 8 ] = {
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

        static sign op_add ( sr a, sr b ) {
            return add_lut[ a->value ][ b->value ];;
        }
        static sign op_fadd( sr, sr ) { return {}; }
        static sign op_sub ( sr, sr ) { return {}; }
        static sign op_fsub( sr, sr ) { return {}; }
        static sign op_mul ( sr, sr ) { return {}; }
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

        static sign op_eq ( sr, sr ) { return {}; }
        static sign op_ne ( sr, sr ) { return {}; }
        static sign op_ugt( sr, sr ) { return {}; }
        static sign op_uge( sr, sr ) { return {}; }
        static sign op_ult( sr, sr ) { return {}; }
        static sign op_ule( sr, sr ) { return {}; }
        static sign op_sgt( sr, sr ) { return {}; }
        static sign op_sge( sr, sr ) { return {}; }
        static sign op_slt( sr, sr ) { return {}; }
        static sign op_sle( sr, sr ) { return {}; }

        static sign op_ffalse( sr, sr ) { return {}; }
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
                case se::top:  return "⊥";
            }

            __builtin_unreachable();
        }

        template< typename stream >
        friend stream& operator<<( stream &os, sr v ) { return os << trace(v); }
    };

} // namespace __lava
