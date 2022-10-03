
#include <lava/support/base.hpp>
#include <lava/support/scalar.hpp>
#include <lava/support/reference.hpp>
#include <lava/constant.hpp>

#include <lamp/support/semilattice.hpp>

#include <sstream>

namespace __lava
{
    struct bitvec_interval_storage {
        std::uint64_t left, right;
        std::uint8_t bw;
        bool is_signed = false;
    };

    template< template< typename > typename storage >
    struct bitvec_interval : storage< bitvec_interval_storage >
                , domain_mixin< bitvec_interval< storage > >
    {
        using base = storage< bitvec_interval_storage >;
        using mixin = domain_mixin< bitvec_interval >;

        using bw = typename mixin::bw;
        using base::base;

        using bvi = bitvec_interval;
        using bitvec_interval_ref = const bitvec_interval &;

        template< typename type > static bitvec_interval lift( const type& ) { return {}; }
        template< typename type > static bitvec_interval any() { return {}; }

        template< typename size >
        static bitvec_interval op_alloca( const size&, uint8_t ) { return {}; }

        static bitvec_interval op_load( bitvec_interval_ref, uint8_t ) { return {}; }
        static bitvec_interval op_load_at( bitvec_interval_ref, bitvec_interval_ref, uint8_t ) { return {}; }

        template< typename scalar >
        static void op_store( bitvec_interval_ref, const scalar&, uint8_t ) {}

        static void assume( bitvec_interval_ref, bool ) {}

        static tristate to_tristate( bitvec_interval_ref ) { return maybe; }

        /* arithmetic operations */
        static bitvec_interval op_add ( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_fadd( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_sub ( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_fsub( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_mul ( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_fmul( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_udiv( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_sdiv( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_fdiv( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_urem( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_srem( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_frem( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }

        static bitvec_interval op_fneg( bitvec_interval_ref ) { return {}; }

        /* bitwise operations */
        static bitvec_interval op_shl ( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_lshr( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_ashr( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_and ( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_or  ( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_xor ( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }

        /* comparison operations */
        static bitvec_interval op_foeq( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_fogt( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_foge( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_folt( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_fole( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_fone( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_ford( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_funo( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_fueq( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_fugt( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_fuge( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_fult( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_fule( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_fune( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }

        static bitvec_interval op_eq ( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_ne ( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_ugt( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_uge( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_ult( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_ule( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_sgt( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_sge( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_slt( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_sle( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }

        static bitvec_interval op_ffalse( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }
        static bitvec_interval op_ftrue( bitvec_interval_ref, bitvec_interval_ref ) { return {}; }


        static bitvec_interval op_fpext( bitvec_interval_ref, bw ) { return {}; }
        static bitvec_interval op_fptosi( bitvec_interval_ref, bw ) { return {}; }
        static bitvec_interval op_fptoui( bitvec_interval_ref, bw ) { return {}; }
        static bitvec_interval op_fptrunc( bitvec_interval_ref, bw ) { return {}; }
        static bitvec_interval op_inttoptr( bitvec_interval_ref, bw ) { return {}; }
        static bitvec_interval op_ptrtoint( bitvec_interval_ref, bw ) { return {}; }
        static bitvec_interval op_sext( bitvec_interval_ref, bw ) { return {}; }
        static bitvec_interval op_sitofp( bitvec_interval_ref, bw ) { return {}; }
        static bitvec_interval op_trunc( bitvec_interval_ref, bw ) { return {}; }
        static bitvec_interval op_uitofp( bitvec_interval_ref, bw ) { return {}; }
        static bitvec_interval op_zext( bitvec_interval_ref, bw ) { return {}; }
        static bitvec_interval op_zfit( bitvec_interval_ref, bw ) { return {}; }

        static std::string trace( bitvec_interval_ref ) { return "bitvec_interval"; }

        template< typename stream >
        friend stream& operator<<( stream &os, bitvec_interval_ref i ) { return os << trace( i ); }
    };
} // namespace __lava
