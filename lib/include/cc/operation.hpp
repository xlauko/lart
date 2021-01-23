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

#include <cc/dfa.hpp>

#include <llvm/IR/Value.h>
#include <variant>
namespace lart::op
{
    struct base
    {
        base( llvm::Value *what )
            : base( what, llvm::cast< llvm::Instruction >( what ) )
        {}

        base( llvm::Value * what, llvm::Instruction *where )
            : _what( what ), _where( where )
        {}

        llvm::Instruction * location() { return _where; }

        llvm::Value * _what;
        llvm::Instruction * _where;
    };

    struct melt : base
    {
        std::string name() const { return "melt"; }
    };

    struct freeze : base
    {
        std::string name() const { return "freeze"; }
    };

    struct binary : base
    {
        std::string name() const { return "bin"; }
    };

    struct alloc : base
    {
        std::string name() const { return "alloca"; }
    };

    struct store : base
    {
        std::string name() const { return "store"; }
    };

    struct load : base
    {
        std::string name() const { return "load"; }
    };

    using operation = std::variant<
        melt, freeze,
        binary,
        alloc, store, load >;

    namespace detail
    {
        static auto invoke = [] (auto f) { return [=] ( auto a ) { return std::visit(f, a); }; };
    } // namespace detail

    static auto location = detail::invoke( [] (auto o) { return o.location(); } );
    static auto name = detail::invoke( [] (const auto &o) { return o.name(); } );

} // namespace lart::op
