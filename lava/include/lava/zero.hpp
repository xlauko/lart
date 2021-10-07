#include <lava/support/base.hpp>
#include <lava/support/tristate.hpp>

namespace __lava
{
    struct [[gnu::packed]] zero_storage
    {
        using underlying_type = uint8_t;
        enum value_type : underlying_type { zero = 0, nonzero = 1, unknown = 2, bottom = 3 } value;

        constexpr zero_storage() : value( unknown ) {}

        constexpr zero_storage( value_type v ) : value( v ) {}
        constexpr zero_storage( uint8_t v ) : zero_storage( value_type( v ) ) {}
    };

    template< template< typename > typename storage >
    struct zero : storage< zero_storage > 
                , domain_mixin< zero< storage > >
    {
        using base = storage< zero_storage >;
        using mixin = domain_mixin< zero >;

        using bw = typename mixin::bw;
        using base::base;

        using ze = zero_storage;
        using zv = zero;
        using zr = const zero &;

        __inline ze::value_type value() const { return this->get().value; }

        __inline bool is_bottom()  const { return value() == ze::bottom; }
        __inline bool is_zero()    const { return value() == ze::zero; }

        static zero top()    { return ze::unknown; }
        static zero bottom() { return ze::bottom; }
    
        template< typename type >
        static zero lift( const type &v ) {
            if constexpr ( std::is_integral_v< type > || std::is_pointer_v< type > )
                return v ? ze::nonzero : ze::zero;
            else
                mixin::fail( "null pointer dereference" );
            return ze::unknown;
        }

        template< typename type > static zero any() { return ze::unknown; }

        /* TODO: implement */
        template< typename size >
        static zero op_alloca( const size&, uint8_t ) { return {}; }

        static zero op_load( zr p, uint8_t )
        {
            if ( p->value != ze::nonzero )
                mixin::fail( "null pointer dereference" );
            return ze::unknown;
        }

        /* TODO: implement */
        static zero op_load_at( zr, zr, uint8_t ) { return {}; }

        template< typename scalar >
        static void op_store( zr p, const scalar&, uint8_t )
        {
            if ( p->value != ze::nonzero )
                mixin::fail( "null pointer dereference" );
        }

        /* TODO: implement */
        template< typename scalar >
        static void op_store_at( zr, zr, const scalar&, uint8_t );

        void intersect( ze::value_type b )
        {
            auto &val = this->get().value;

            if ( val == ze::zero )
                val = b != ze::nonzero ? ze::zero : ze::bottom;
            else if ( val == ze::nonzero )
                val = b != ze::zero ? ze::nonzero : ze::bottom;
            else
                val = b;

            if ( is_bottom() )
                __lart_cancel();
        }

        static void assume( zero &z, bool constraint )
        {
            z.intersect( constraint ? ze::nonzero : ze::zero );
            if ( z.is_bottom() )
                __lart_cancel();
        }

        static tristate to_tristate( zr z )
        {
            switch ( z->value )
            {
            case ze::zero:    return tristate::false_value;
            case ze::nonzero: return tristate::true_value;
            case ze::unknown: return tristate::maybe_value;
            case ze::bottom: __lart_cancel();
            }

            __builtin_unreachable();
        }

        static constexpr ze::value_type add_lut[ 3 ][ 3 ] = {
            { ze::zero,    ze::nonzero, ze::unknown },
            { ze::nonzero, ze::unknown, ze::unknown },
            { ze::unknown, ze::unknown, ze::unknown }
        };

        static constexpr ze::value_type mul_lut[ 3 ][ 3 ] = {
            { ze::zero, ze::zero,    ze::zero    },
            { ze::zero, ze::unknown, ze::unknown },
            { ze::zero, ze::unknown, ze::unknown }
        };

        static zv div( zr a, zr b )
        {
            if ( b->value != ze::nonzero )
                mixin::fail( "division by zero" );
            return a->value ? ze::unknown : ze::zero;
        }

        static constexpr ze::value_type shift_lut[ 3 ][ 3 ] = {
            { ze::zero,    ze::zero,    ze::zero    },
            { ze::nonzero, ze::unknown, ze::unknown },
            { ze::unknown, ze::unknown, ze::unknown }
        };

        static constexpr ze::value_type and_lut[ 3 ][ 3 ] = {
            { ze::zero, ze::zero,    ze::zero    },
            { ze::zero, ze::unknown, ze::unknown },
            { ze::zero, ze::unknown, ze::unknown }
        };

        static constexpr ze::value_type or_lut[ 3 ][ 3 ] = {
            { ze::zero,    ze::nonzero, ze::unknown },
            { ze::nonzero, ze::nonzero, ze::nonzero },
            { ze::unknown, ze::nonzero, ze::unknown }
        };

        static constexpr ze::value_type xor_lut[ 3 ][ 3 ] = {
            { ze::zero,    ze::nonzero, ze::unknown },
            { ze::nonzero, ze::unknown, ze::unknown },
            { ze::unknown, ze::unknown, ze::unknown }
        };

        static constexpr ze::value_type eq_lut[ 3 ][ 3 ] = {
            { ze::nonzero, ze::zero,    ze::unknown },
            { ze::zero,    ze::unknown, ze::unknown },
            { ze::unknown, ze::unknown, ze::unknown }
        };

        static constexpr ze::value_type ne_lut[ 3 ][ 3 ] = {
            { ze::zero,    ze::nonzero, ze::unknown },
            { ze::nonzero, ze::unknown, ze::unknown },
            { ze::unknown, ze::unknown, ze::unknown }
        };

        static constexpr ze::value_type ugt_lut[ 3 ][ 3 ] = {
            { ze::zero,    ze::zero,    ze::zero    },
            { ze::nonzero, ze::unknown, ze::unknown },
            { ze::unknown, ze::unknown, ze::unknown }
        };

        static constexpr ze::value_type uge_lut[ 3 ][ 3 ] = {
            { ze::nonzero, ze::zero,    ze::unknown },
            { ze::nonzero, ze::unknown, ze::unknown },
            { ze::nonzero, ze::unknown, ze::unknown }
        };

        static constexpr ze::value_type sgt_lut[ 3 ][ 3 ] = {
            { ze::zero,    ze::unknown, ze::unknown },
            { ze::unknown, ze::unknown, ze::unknown },
            { ze::unknown, ze::unknown, ze::unknown }
        };

        static constexpr ze::value_type sge_lut[ 3 ][ 3 ] = {
            { ze::nonzero, ze::unknown, ze::unknown },
            { ze::unknown, ze::unknown, ze::unknown },
            { ze::unknown, ze::unknown, ze::unknown }
        };

        /* arithmetic operations */
        static zero op_add ( zr a, zr b ) { return add_lut[ a->value ][ b->value ]; }
        static zero op_sub ( zr a, zr b ) { return add_lut[ a->value ][ b->value ]; }
        static zero op_mul ( zr a, zr b ) { return add_lut[ a->value ][ b->value ]; }
        static zero op_udiv( zr a, zr b ) { return div( a, b ); }
        static zero op_sdiv( zr a, zr b ) { return div( a, b ); }
        static zero op_urem( zr a, zr b ) { return div( a, b ); }
        static zero op_srem( zr a, zr b ) { return div( a, b ); }
    
        /* TODO: implement float arithmetic operations */
        static zero op_fadd( zr, zr ) { return {}; }
        static zero op_fsub( zr, zr ) { return {}; }
        static zero op_fmul( zr, zr ) { return {}; }
        static zero op_fdiv( zr, zr ) { return {}; }
        static zero op_frem( zr, zr ) { return {}; }
        static zero op_fneg( zr ) { return {}; }

        /* bitwise operations */
        static zv op_shl ( zr a, zr b ) { return shift_lut[ a->value ][ b->value ]; }
        static zv op_ashr( zr a, zr b ) { return shift_lut[ a->value ][ b->value ]; }
        static zv op_lshr( zr a, zr b ) { return shift_lut[ a->value ][ b->value ]; }
        static zv op_and ( zr a, zr b ) { return and_lut  [ a->value ][ b->value ]; }
        static zv op_or  ( zr a, zr b ) { return or_lut   [ a->value ][ b->value ]; }
        static zv op_xor ( zr a, zr b ) { return xor_lut  [ a->value ][ b->value ]; }

        /* comparison operations */
        static zv op_eq ( zr a, zr b ) { return eq_lut[ a->value ][ b->value ]; }
        static zv op_ne ( zr a, zr b ) { return ne_lut[ a->value ][ b->value ]; }
        static zv op_ugt( zr a, zr b ) { return ugt_lut[ a->value ][ b->value ]; }
        static zv op_uge( zr a, zr b ) { return uge_lut[ a->value ][ b->value ]; }
        static zv op_ult( zr a, zr b ) { return ugt_lut[ b->value ][ a->value ]; }
        static zv op_ule( zr a, zr b ) { return uge_lut[ b->value ][ a->value ]; }
        static zv op_sgt( zr a, zr b ) { return sgt_lut[ a->value ][ b->value ]; }
        static zv op_sge( zr a, zr b ) { return sge_lut[ a->value ][ b->value ]; }
        static zv op_slt( zr a, zr b ) { return sgt_lut[ b->value ][ a->value ]; }
        static zv op_sle( zr a, zr b ) { return sge_lut[ b->value ][ a->value ]; }

        /* TODO: implement float comparison operations */
        static zero op_foeq( zr, zr ) { return {}; }
        static zero op_fogt( zr, zr ) { return {}; }
        static zero op_foge( zr, zr ) { return {}; }
        static zero op_folt( zr, zr ) { return {}; }
        static zero op_fole( zr, zr ) { return {}; }
        static zero op_fone( zr, zr ) { return {}; }
        static zero op_ford( zr, zr ) { return {}; }
        static zero op_funo( zr, zr ) { return {}; }
        static zero op_fueq( zr, zr ) { return {}; }
        static zero op_fugt( zr, zr ) { return {}; }
        static zero op_fuge( zr, zr ) { return {}; }
        static zero op_fult( zr, zr ) { return {}; }
        static zero op_fule( zr, zr ) { return {}; }
        static zero op_fune( zr, zr ) { return {}; }

        static zero op_ffalse( zr, zr ) { return {}; }
        static zero op_ftrue( zr, zr ) { return {}; }

        static zero op_fpext( zr, bw ) { return {}; }
        static zero op_fptosi( zr, bw ) { return {}; }
        static zero op_fptoui( zr, bw ) { return {}; }
        static zero op_fptrunc( zr, bw ) { return {}; }
        static zero op_inttoptr( zr, bw ) { return {}; }
        static zero op_ptrtoint( zr, bw ) { return {}; }
        static zero op_sext( zr z, bw ) { return z.clone(); }
        static zero op_sitofp( zr, bw ) { return {}; }
        static zero op_trunc( zr z, bw ) { return z.is_zero() ? ze::zero : ze::unknown; }
        static zero op_uitofp( zr, bw ) { return {}; }
        static zero op_zext( zr z, bw ) { return z.clone(); }
        static zero op_zfit( zr z, bw ) { return z.is_zero() ? ze::zero : ze::unknown; }

        /* TODO: import bops from old zero domain */

        static const char *str( zr z )
        {
            switch ( z.get().value ) {
                case ze::zero:    return "Z";
                case ze::nonzero: return "N";
                case ze::unknown: return "U";
                case ze::bottom:  return "⊥";
            }

            __builtin_unreachable();
        }

        static void dump( zr z )
        {
            printf( "%s\n", str( z ) );
        }

        static std::string trace( zr z ) { return str( z ); }

        template< typename stream >
        friend stream& operator<<( stream &os, zr z )
        {
            return os << str( z );
        }
    };
}
