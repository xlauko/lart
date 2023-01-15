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

#include <lava/support/tristate.hpp>
#include <lava/support/base.hpp>
#include <lamp/support/tracing.hpp>

#include <sys/mman.h>

#include <iostream>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <type_traits>
#include <cvc5/cvc5.h>

using namespace std::string_literals;
namespace cvc = cvc5::api;

namespace __lava
{
    struct term_config_t
    {
        bool trace_model = false;
    };

    struct term_state_t
    {
        term_state_t()
        {
            solver.setLogic("QF_BV");
            solver.setOption("produce-models", "true");
        }

        std::vector< cvc::Term > vars;
        cvc::Solver solver;
    };

    term_state_t *__term_state;
    term_config_t *__term_cfg;

    template< template< typename > typename storage >
    struct term : storage< cvc::Term >
                , domain_mixin< term< storage > >
    {
        using base = storage< cvc::Term >;
        using mixin = domain_mixin< term >;

        using bw = typename mixin::bw;
        using base::base;

        using tr = const term &;

        static cvc::Solver& solver() { return __term_state->solver; }
        static std::vector< cvc::Term >& variables() { return __term_state->vars; }

        template< typename type > static term lift( const type &value )
        {
            auto &slv = solver();

            if constexpr ( std::is_integral_v < type > )
            {
                return slv.mkBitVector(bitwidth_v< type >, value );
            }

            __builtin_unreachable();
        }

        template< typename type > static term any()
        {
            auto &slv = solver();
            auto &vars = variables();

            std::string name = "var_" + std::to_string( vars.size() );
            if constexpr ( std::is_integral_v < type > )
            {
                auto sort = slv.mkBitVectorSort( bitwidth_v< type > );
                vars.push_back( slv.mkConst( sort, name ) );
                return vars.back();
            }

            // if constexpr ( std::is_floating_point_v< type > )
            // {
            //     if constexpr ( bitwidth_v< type > == 32 )
            //         return ctx.fpa_const( name, 8, 24 );
            //     if constexpr ( bitwidth_v< type > == 64 )
            //         return ctx.fpa_const( name, 11, 53 );
            // }
            __builtin_unreachable();
        }


        template< typename type >
        static term any(const variadic_list & /* args */) {
            mixin::fail("unsupported variadic any operation");
        }

        template< typename type >
        static term any(type from, type to) {
            mixin:fail("unsupported range any"):
        }

        static cvc::Term tobool( const cvc::Term &e )
        {
            assert( e.isBitVectorValue() );
            assert( e.getSort().getBVSize() == 1 );
            auto &slv = solver();
            return slv.mkTerm( cvc::EQUAL, e, slv.mkBitVector( 1, 1 ) );
        }

        static void assume( tr t, bool expected )
        {
            auto &slv = solver();
            const auto& e = t.get();

            auto b = e.getSort().isBoolean() ? e : tobool( e );
            if (expected)
                slv.assertFormula( b );
            else
                slv.assertFormula( b.notTerm() );

            if ( slv.checkSat().isUnsat() ) {
                __lart_cancel();
            }
        }

        static tristate to_tristate( tr ) { return maybe; }

        /* arithmetic operations */
        static term op_add ( tr a, tr b )
        {
            return solver().mkTerm( cvc::BITVECTOR_ADD, a.get(), b.get() );
        }

        static term op_sub ( tr a, tr b )
        {
            return solver().mkTerm( cvc::BITVECTOR_SUB, a.get(), b.get() );
        }

        static term op_mul ( tr a, tr b )
        {
            return solver().mkTerm( cvc::BITVECTOR_MULT, a.get(), b.get() );
        }

        static term op_udiv( tr a, tr b )
        {
            return solver().mkTerm( cvc::BITVECTOR_UDIV, a.get(), b.get() );
        }

        static term op_sdiv( tr a, tr b )
        {
            return solver().mkTerm( cvc::BITVECTOR_SDIV, a.get(), b.get() );
        }

        static term op_urem( tr a, tr b )
        {
            return solver().mkTerm( cvc::BITVECTOR_UREM, a.get(), b.get() );
        }

        static term op_srem( tr a, tr b )
        {
            return solver().mkTerm( cvc::BITVECTOR_SREM, a.get(), b.get() );
        }

        // /* bitwise operations */
        static term op_shl ( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_SHL, a.get(), b.get() );
        }

        static term op_lshr( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_LSHR, a.get(), b.get() );
        }

        static term op_ashr( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_ASHR, a.get(), b.get() );
        }

        static term op_and ( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_AND, a.get(), b.get() );
        }

        static term op_or( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_OR, a.get(), b.get() );
        }

        static term op_xor( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_XOR, a.get(), b.get() );
        }

        // /* comparison operations */
        static term op_eq ( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_COMP, a.get(), b.get() );
        }

        static term op_ne ( tr a, tr b )
        {
            return solver().mkTerm( cvc::DISTINCT, a.get(), b.get() );
        }

        static term op_ugt( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_UGT, a.get(), b.get() );
        }

        static term op_uge( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_UGE, a.get(), b.get() );
        }

        static term op_ult( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_ULT, a.get(), b.get() );
        }

        static term op_ule( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_ULE, a.get(), b.get() );
        }

        static term op_sgt( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_SGT, a.get(), b.get() );
        }

        static term op_sge( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_SGE, a.get(), b.get() );
        }

        static term op_slt( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_SLT, a.get(), b.get() );
        }

        static term op_sle( tr a, tr b )
        {
           return solver().mkTerm( cvc::BITVECTOR_SLE, a.get(), b.get() );
        }

        // static term op_inttoptr( tr, bw ) { return {}; }
        // static term op_ptrtoint( tr, bw ) { return {}; }
        // static term op_sext( tr t, bw b )
        // {
        //     auto &v = t.get();
        //     auto op = solver().mkOp( cvc::BITVECTOR_SIGN_EXTEND, b - v.getSort().getBVSize() );
        //     return solver().mkTerm( op, v );
        // }
        // // static term op_sitofp( tr, bw ) { return {}; }
        // static term op_trunc( tr t, bw b )
        // {
        //     auto &v = t.get();
        //     auto op = solver().mkOp( cvc::BITVECTOR_EXTRACT, b - 1, 0 );
        //     return solver().mkTerm( op, v );
        // }
        // // static term op_uitofp( tr, bw ) { return {}; }
        // static term op_zext( tr t, bw b )
        // {
        //     auto &v = t.get();
        //     auto op = solver().mkOp( cvc::BITVECTOR_ZERO_EXTEND, b - v.getSort().getBVSize() );
        //     return solver().mkTerm( op, v );
        // }
        // static term op_zfit( tr t, bw ) { return {}; }

        static std::string trace( tr t )
        {
            return t.get().toString();
        }

        template< typename stream >
        friend stream& operator<<( stream &os, tr t )
        {
            return os << trcae(t);
        }
    };


    inline void trace_model()
    {
        auto &slv = __term_state->solver;
        auto &vars = __term_state->vars;

        for (const auto &var : vars) {
            std::cerr << "[term model] " << "TODO NAME" << " = " << slv.getValue(var) << '\n';
        }
    }

    inline bool option( std::string_view option, std::string_view msg )
    {
        auto is_set = [] ( auto opt ) { return opt && strcmp( opt, "ON" ) == 0; };
        if ( auto opt = std::getenv( option.data() ); is_set( opt ) ) {
            fprintf( stderr, "[term config] %s\n", msg.data() );
            return  true;
        }
        return false;
    }

    [[gnu::constructor]] void term_setup()
    {
        __term_state = (term_state_t*)std::malloc(sizeof(term_state_t));
        new ( __term_state ) term_state_t;

        __term_cfg = (term_config_t*)mmap(NULL, sizeof(term_config_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        __term_cfg->trace_model = option("TERM_TRACE_MODEL", "term trace model");
    }

    [[gnu::destructor]] void term_cleanup()
    {
        if ( __term_cfg->trace_model ) {
            trace_model();
            // do not trace model multiple times
            __term_cfg->trace_model = false;
        }

        std::free( __term_state );
        munmap( __term_cfg, sizeof(term_config_t) );
    }

} // namespace __lava
