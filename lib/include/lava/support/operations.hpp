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

namespace __lava::op
{
    static constexpr auto lower  = []( auto a ) { return decltype( a )::lower;  };

    static constexpr auto _alloca = []( auto a )
    {
        return []( const auto & ... x ) { return decltype( a )::op_alloca( x... ); };
    };

    static constexpr auto join = []( auto a ) { return decltype( a )::op_join; };
    static constexpr auto meet = []( auto a ) { return decltype( a )::op_meet; };

    static constexpr auto add = []( auto a ) { return decltype( a )::op_add; };
    static constexpr auto sub = []( auto a ) { return decltype( a )::op_sub; };
    static constexpr auto mul = []( auto a ) { return decltype( a )::op_mul; };

    static constexpr auto sdiv = []( auto a ) { return decltype( a )::op_sdiv; };
    static constexpr auto udiv = []( auto a ) { return decltype( a )::op_udiv; };
    static constexpr auto srem = []( auto a ) { return decltype( a )::op_srem; };
    static constexpr auto urem = []( auto a ) { return decltype( a )::op_urem; };

    static constexpr auto fadd = []( auto a ) { return decltype( a )::op_fadd; };
    static constexpr auto fsub = []( auto a ) { return decltype( a )::op_fsub; };
    static constexpr auto fmul = []( auto a ) { return decltype( a )::op_fmul; };
    static constexpr auto fdiv = []( auto a ) { return decltype( a )::op_fdiv; };
    static constexpr auto frem = []( auto a ) { return decltype( a )::op_frem; };

    static constexpr auto _and = []( auto a ) { return decltype( a )::op_and; };
    static constexpr auto _or = []( auto a ) { return decltype( a )::op_or; };
    static constexpr auto _xor = []( auto a ) { return decltype( a )::op_xor; };

    static constexpr auto zfit  = []( auto a ) { return decltype( a )::op_zfit; };
    static constexpr auto zext  = []( auto a ) { return decltype( a )::op_zext; };
    static constexpr auto sext  = []( auto a ) { return decltype( a )::op_sext; };
    static constexpr auto trunc = []( auto a ) { return decltype( a )::op_trunc; };

    static constexpr auto fptrunc = []( auto a ) { return decltype( a )::op_fptrunc; };
    static constexpr auto sitofp  = []( auto a ) { return decltype( a )::op_sitofp; };
    static constexpr auto uitofp  = []( auto a ) { return decltype( a )::op_uitofp; };
    static constexpr auto fptosi  = []( auto a ) { return decltype( a )::op_fptosi; };
    static constexpr auto fptoui  = []( auto a ) { return decltype( a )::op_fptoui; };
    static constexpr auto fpext   = []( auto a ) { return decltype( a )::op_fpext; };

    static constexpr auto ne   = []( auto a ) { return decltype( a )::op_ne; };
    static constexpr auto eq   = []( auto a ) { return decltype( a )::op_eq; };

    static constexpr auto slt  = []( auto a ) { return decltype( a )::op_slt; };
    static constexpr auto sgt  = []( auto a ) { return decltype( a )::op_sgt; };
    static constexpr auto sle  = []( auto a ) { return decltype( a )::op_sle; };
    static constexpr auto sge  = []( auto a ) { return decltype( a )::op_sge; };

    static constexpr auto ult  = []( auto a ) { return decltype( a )::op_ult; };
    static constexpr auto ugt  = []( auto a ) { return decltype( a )::op_ugt; };
    static constexpr auto ule  = []( auto a ) { return decltype( a )::op_ule; };
    static constexpr auto uge  = []( auto a ) { return decltype( a )::op_uge; };

    static constexpr auto foeq  = []( auto a ) { return decltype( a )::op_foeq; };
    static constexpr auto fogt  = []( auto a ) { return decltype( a )::op_fogt; };
    static constexpr auto foge  = []( auto a ) { return decltype( a )::op_foge; };
    static constexpr auto folt  = []( auto a ) { return decltype( a )::op_folt; };
    static constexpr auto fole  = []( auto a ) { return decltype( a )::op_fole; };
    static constexpr auto fone  = []( auto a ) { return decltype( a )::op_fone; };
    static constexpr auto ford  = []( auto a ) { return decltype( a )::op_ford; };
    static constexpr auto funo  = []( auto a ) { return decltype( a )::op_funo; };
    static constexpr auto fueq  = []( auto a ) { return decltype( a )::op_fueq; };
    static constexpr auto fugt  = []( auto a ) { return decltype( a )::op_fugt; };
    static constexpr auto fuge  = []( auto a ) { return decltype( a )::op_fuge; };
    static constexpr auto fult  = []( auto a ) { return decltype( a )::op_fult; };
    static constexpr auto fule  = []( auto a ) { return decltype( a )::op_fule; };
    static constexpr auto fune  = []( auto a ) { return decltype( a )::op_fune; };

    static constexpr auto ffalse  = []( auto a ) { return decltype( a )::op_ffalse; };
    static constexpr auto ftrue  = []( auto a ) { return decltype( a )::op_ftrue; };

    static constexpr auto shl  = []( auto a ) { return decltype( a )::op_shl; };
    static constexpr auto lshr = []( auto a ) { return decltype( a )::op_lshr; };
    static constexpr auto ashr = []( auto a ) { return decltype( a )::op_ashr; };

    static constexpr auto load = []( auto a ) { return decltype( a )::op_load; };
    static constexpr auto load_at = []( auto a ) { return decltype( a )::op_load_at; };

    static constexpr auto store = []( auto a )
    {
        return []( const auto & ... x ) { return decltype( a )::op_store( x... ); };
    };

    static constexpr auto store_at = []( auto a )
    {
        return []( const auto & ... x ) { return decltype( a )::op_store_at( x... ); };
    };

    static constexpr auto concat  = []( auto a ) { return decltype( a )::op_concat; };
    static constexpr auto extract = []( auto a )
    {
        return []( const auto & ... x ) { return decltype( a )::op_extract( x... ); };
    };

    static constexpr auto assume = []( auto a ) { return decltype( a )::assume; };
    static constexpr auto to_tristate = []( auto a ) { return decltype( a )::to_tristate; };

    static constexpr auto strcmp = []( auto a ) { return decltype( a )::fn_strcmp; };
    static constexpr auto strlen = []( auto a ) { return decltype( a )::fn_strlen; };
    static constexpr auto strcat = []( auto a ) { return decltype( a )::fn_strcat; };
    static constexpr auto strcpy = []( auto a ) { return decltype( a )::fn_strcpy; };

    static constexpr auto strchr = []( auto a )
    {
        return []( const auto & ... x ) { return decltype( a )::fn_strchr( x... ); };
    };

    static constexpr auto memcpy = []( auto a )
    {
        return []( const auto & ... x ) { return decltype( a )::fn_memcpy( x... ); };
    };

    static constexpr auto malloc = []( auto a )
    {
        return []( const auto & ... x ) { return decltype( a )::fn_malloc( x... ); };
    };

    static constexpr auto realloc = []( auto a )
    {
        return []( const auto & ... x ) { return decltype( a )::fn_realloc( x... ); };
    };

    static constexpr auto dealloca = []( auto a ) { return decltype( a )::op_dealloca; };
    static constexpr auto free = []( auto a ) { return decltype( a )::fn_free; };
} // namespace __lava::op