/*
 * (c) 2020 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <string_view>

namespace __lava::op
{
    enum class tag
    {
        unknown,
        any, lift, lower,
        join, meet,
        
        alloca, store, load,

        add, sub, mul, sdiv, udiv, srem, urem,
        fadd, fsub, fmul, fdiv, frem,

        fneg,
        
        op_and, op_or, op_xor,

        zfit, zext, sext, trunc,

        fptrunc, sitofp, uitofp, fptosi, fptoui, fpext,
        inttoptr, ptrtoint,

        ne, eq, slt, sgt, sle, sge, ult, ugt, ule, uge,

        foeq, fogt, foge, folt, fole, fone, ford, 
        funo, fueq, fugt, fuge, fult, fule, fune,

        ffalse, ftrue,

        shl, lshr, ashr, concat, extract,

        malloc, realloc, dealloca, free,

        abs,

        fabs, round, copysign, fmin, fmax, rint, fmod,
        ceil, isinf, isnan, isfinite, fntrunc
    };

    std::string_view to_string(tag t)
    {
        switch (t)
        {
            case tag::unknown: return "unknown";

            case tag::any:   return "any";
            case tag::lift:  return "lift";
            case tag::lower: return "lower";
            case tag::join:  return "join";
            case tag::meet:  return "meet";
            
            case tag::alloca: return "alloca";
            case tag::store:  return "store";
            case tag::load:   return "load";

            case tag::add: return "add";
            case tag::sub: return "sub";
            case tag::mul: return "mul";
            case tag::sdiv: return "sdiv";
            case tag::udiv: return "udiv";
            case tag::srem: return "srem";
            case tag::urem: return "urem";

            case tag::fadd: return "fadd";
            case tag::fsub: return "fsub";
            case tag::fmul: return "fmul";
            case tag::fdiv: return "fdiv";
            case tag::frem: return "frem";

            case tag::fneg: return "fneg";
            
            case tag::op_and: return "op_and";
            case tag::op_or: return "op_or";
            case tag::op_xor: return "op_xor";

            case tag::zfit: return "zfit";
            case tag::zext: return "zext"; 
            case tag::sext: return "sext"; 
            case tag::trunc: return "trunc";

            case tag::fptrunc: return "fptrunc";
            case tag::sitofp: return "sitofp";
            case tag::uitofp: return "uitofp";
            case tag::fptosi: return "fptosi";
            case tag::fptoui: return "fptoui";
            case tag::fpext: return "fpext";
            case tag::inttoptr: return "inttoptr";
            case tag::ptrtoint: return "ptrtoint";

            case tag::ne: return "ne";
            case tag::eq: return "eq";
            case tag::slt: return "slt";
            case tag::sgt: return "sgt";
            case tag::sle: return "sle";
            case tag::sge: return "sge";
            case tag::ult: return "ult";
            case tag::ugt: return "ugt";
            case tag::ule: return "ule";
            case tag::uge: return "uge";

            case tag::foeq: return "foeq";
            case tag::fogt: return "fogt";
            case tag::foge: return "foge";
            case tag::folt: return "folt";
            case tag::fole: return "fole";
            case tag::fone: return "fone";
            case tag::ford: return "ford"; 
            case tag::funo: return "funo";
            case tag::fueq: return "fueq";
            case tag::fugt: return "fugt";
            case tag::fuge: return "fuge";
            case tag::fult: return "fult";
            case tag::fule: return "fule";
            case tag::fune: return "fune";

            case tag::ffalse: return "ffalse";
            case tag::ftrue: return "ftrue";

            case tag::shl: return "shl";
            case tag::lshr: return "lshr";
            case tag::ashr: return "ashr";
            case tag::concat: return "concat";
            case tag::extract: return "extract";

            case tag::malloc: return "malloc";
            case tag::realloc: return "realloc";
            
            case tag::dealloca: return "dealloca";
            case tag::free : return "free";
            
            case tag::abs: return "abs";

            case tag::fabs: return "fabs";
            case tag::round: return "round";
            case tag::ceil: return "ceil";
            case tag::fntrunc: return "fntrunc";
            case tag::isnan: return "isnan";
            case tag::isinf: return "isinf";
            case tag::isfinite: return "isfinite";
            case tag::copysign: return "copysign";
            case tag::fmod: return "fmod";
            case tag::fmax: return "fmax";
            case tag::fmin: return "fmin";
            case tag::rint: return "rint";
        }

        __builtin_unreachable();
    }

    static constexpr auto lower  = []( const auto &a ) { return std::decay_t< decltype( a ) >::lower;  };

    static constexpr auto _alloca = []( const auto &a )
    {
        return []( const auto & ... x ) { return std::decay_t< decltype( a ) >::op_alloca( x... ); };
    };

    static constexpr auto join = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_join; };
    static constexpr auto meet = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_meet; };

    static constexpr auto fneg = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fneg; };

    static constexpr auto add = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_add; };
    static constexpr auto sub = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_sub; };
    static constexpr auto mul = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_mul; };

    static constexpr auto sdiv = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_sdiv; };
    static constexpr auto udiv = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_udiv; };
    static constexpr auto srem = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_srem; };
    static constexpr auto urem = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_urem; };

    static constexpr auto fadd = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fadd; };
    static constexpr auto fsub = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fsub; };
    static constexpr auto fmul = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fmul; };
    static constexpr auto fdiv = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fdiv; };
    static constexpr auto frem = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_frem; };

    static constexpr auto _and = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_and; };
    static constexpr auto _or = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_or; };
    static constexpr auto _xor = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_xor; };

    static constexpr auto zfit  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_zfit; };
    static constexpr auto zext  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_zext; };
    static constexpr auto sext  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_sext; };
    static constexpr auto trunc = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_trunc; };

    static constexpr auto fptrunc = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fptrunc; };
    static constexpr auto sitofp  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_sitofp; };
    static constexpr auto uitofp  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_uitofp; };
    static constexpr auto fptosi  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fptosi; };
    static constexpr auto fptoui  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fptoui; };
    static constexpr auto fpext   = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fpext; };

    static constexpr auto ne   = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_ne; };
    static constexpr auto eq   = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_eq; };

    static constexpr auto slt  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_slt; };
    static constexpr auto sgt  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_sgt; };
    static constexpr auto sle  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_sle; };
    static constexpr auto sge  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_sge; };

    static constexpr auto ult  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_ult; };
    static constexpr auto ugt  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_ugt; };
    static constexpr auto ule  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_ule; };
    static constexpr auto uge  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_uge; };

    static constexpr auto foeq  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_foeq; };
    static constexpr auto fogt  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fogt; };
    static constexpr auto foge  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_foge; };
    static constexpr auto folt  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_folt; };
    static constexpr auto fole  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fole; };
    static constexpr auto fone  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fone; };
    static constexpr auto ford  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_ford; };
    static constexpr auto funo  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_funo; };
    static constexpr auto fueq  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fueq; };
    static constexpr auto fugt  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fugt; };
    static constexpr auto fuge  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fuge; };
    static constexpr auto fult  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fult; };
    static constexpr auto fule  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fule; };
    static constexpr auto fune  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_fune; };

    static constexpr auto ffalse  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_ffalse; };
    static constexpr auto ftrue  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_ftrue; };

    static constexpr auto shl  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_shl; };
    static constexpr auto lshr = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_lshr; };
    static constexpr auto ashr = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_ashr; };

    static constexpr auto load = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_load; };
    static constexpr auto load_at = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_load_at; };

    static constexpr auto store = []( const auto &a )
    {
        return []( const auto & ... x ) { return std::decay_t< decltype( a ) >::op_store( x... ); };
    };

    static constexpr auto concat  = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_concat; };
    static constexpr auto extract = []( const auto &a )
    {
        return []( const auto & ... x ) { return std::decay_t< decltype( a ) >::op_extract( x... ); };
    };

    static constexpr auto assume = []( const auto &a ) { return std::decay_t< decltype( a ) >::assume; };
    static constexpr auto to_tristate = []( const auto &a ) { return std::decay_t< decltype( a ) >::to_tristate; };
    
    static constexpr auto abs = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_abs; };
    
    static constexpr auto fabs = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_fabs; };
    static constexpr auto round = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_round; };
    static constexpr auto rint = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_rint; };
    static constexpr auto ceil = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_ceil; };
    static constexpr auto fntrunc = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_trunc; };
    static constexpr auto isnan = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_isnan; };
    static constexpr auto isinf = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_isinf; };
    static constexpr auto isfinite = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_isfinite; };
    static constexpr auto copysign = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_copysign; };
    static constexpr auto fmod = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_fmod; };
    static constexpr auto fmax = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_fmax; };
    static constexpr auto fmin = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_fmin; };

    static constexpr auto strcmp = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_strcmp; };
    static constexpr auto strlen = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_strlen; };
    static constexpr auto strcat = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_strcat; };
    static constexpr auto strcpy = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_strcpy; };

    static constexpr auto strchr = []( const auto &a )
    {
        return []( const auto & ... x ) { return std::decay_t< decltype( a ) >::fn_strchr( x... ); };
    };

    static constexpr auto memcpy = []( const auto &a )
    {
        return []( const auto & ... x ) { return std::decay_t< decltype( a ) >::fn_memcpy( x... ); };
    };

    static constexpr auto malloc = []( const auto &a )
    {
        return []( const auto & ... x ) { return std::decay_t< decltype( a ) >::fn_malloc( x... ); };
    };

    static constexpr auto realloc = []( const auto &a )
    {
        return []( const auto & ... x ) { return std::decay_t< decltype( a ) >::fn_realloc( x... ); };
    };

    static constexpr auto dealloca = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_dealloca; };
    static constexpr auto free = []( const auto &a ) { return std::decay_t< decltype( a ) >::fn_free; };


    namespace backward
    {
        static constexpr auto _alloca = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_alloca; };

        static constexpr auto add = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_add; };
        static constexpr auto sub = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_sub; };
        static constexpr auto mul = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_mul; };

        static constexpr auto sdiv = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_sdiv; };
        static constexpr auto udiv = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_udiv; };
        static constexpr auto srem = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_srem; };
        static constexpr auto urem = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_urem; };

        static constexpr auto fadd = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fadd; };
        static constexpr auto fsub = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fsub; };
        static constexpr auto fmul = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fmul; };
        static constexpr auto fdiv = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fdiv; };
        static constexpr auto frem = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_frem; };

        static constexpr auto op_and = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_and; };
        static constexpr auto op_or  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_or; };
        static constexpr auto op_xor = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_xor; };

        static constexpr auto zfit  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_zfit; };
        static constexpr auto zext  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_zext; };
        static constexpr auto sext  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_sext; };
        static constexpr auto trunc = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_trunc; };

        static constexpr auto fptrunc = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fptrunc; };
        static constexpr auto sitofp  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_sitofp; };
        static constexpr auto uitofp  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_uitofp; };
        static constexpr auto fptosi  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fptosi; };
        static constexpr auto fptoui  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fptoui; };
        static constexpr auto fpext   = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fpext; };

        static constexpr auto ne   = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_ne; };
        static constexpr auto eq   = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_eq; };

        static constexpr auto slt  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_slt; };
        static constexpr auto sgt  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_sgt; };
        static constexpr auto sle  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_sle; };
        static constexpr auto sge  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_sge; };

        static constexpr auto ult  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_ult; };
        static constexpr auto ugt  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_ugt; };
        static constexpr auto ule  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_ule; };
        static constexpr auto uge  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_uge; };

        static constexpr auto foeq  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_foeq; };
        static constexpr auto fogt  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fogt; };
        static constexpr auto foge  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_foge; };
        static constexpr auto folt  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_folt; };
        static constexpr auto fole  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fole; };
        static constexpr auto fone  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fone; };
        static constexpr auto ford  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_ford; };
        static constexpr auto funo  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_funo; };
        static constexpr auto fueq  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fueq; };
        static constexpr auto fugt  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fugt; };
        static constexpr auto fuge  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fuge; };
        static constexpr auto fult  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fult; };
        static constexpr auto fule  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fule; };
        static constexpr auto fune  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_fune; };

        static constexpr auto ffalse  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_ffalse; };
        static constexpr auto ftrue  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_ftrue; };

        static constexpr auto shl  = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_shl; };
        static constexpr auto lshr = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_lshr; };
        static constexpr auto ashr = []( const auto &a ) { return std::decay_t< decltype( a ) >::bop_ashr; };
    }

    static constexpr auto run = []( const auto &op, const auto & ... bind  ) __inline
    {
        return [=]( const auto &arg, const auto & ... args ) __inline
        {
            return op( arg )( arg, args..., bind... );
        };
    };

    static constexpr auto wrap = []( const auto &op, const auto & ... bind  ) __inline
    {
        return [=]( const auto &arg, const auto & ... args ) __inline
        {
            return op( arg )( arg, args..., bind... );
        };
    };

    static constexpr auto wrapr = []( const auto &op, const auto & ... bind  ) __inline
    {
        return [=]( auto &arg, const auto & ... args ) __inline
        {
            return op( arg )( arg, args..., bind... );
        };
    };
} // namespace __lava::op