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
    static constexpr auto lower  = []( const auto &a ) { return std::decay_t< decltype( a ) >::lower;  };

    static constexpr auto _alloca = []( const auto &a )
    {
        return []( const auto & ... x ) { return std::decay_t< decltype( a ) >::op_alloca( x... ); };
    };

    static constexpr auto join = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_join; };
    static constexpr auto meet = []( const auto &a ) { return std::decay_t< decltype( a ) >::op_meet; };

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
} // namespace __lava::op