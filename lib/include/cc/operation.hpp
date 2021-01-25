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

#include <sc/ranges.hpp>
#include <sc/constant.hpp>

#include <llvm/IR/Value.h>

#include <variant>
#include <experimental/iterator>
namespace lart::op
{
    inline auto abstract_pointer() { return sc::null( sc::i8p() ); }

    struct base
    {
        base( llvm::Value *what )
            : base( what, llvm::cast< llvm::Instruction >( what ) )
        {}

        base( llvm::Value *what, llvm::Instruction *where )
            : _what( what ), _where( where )
        {}

        llvm::Instruction* location() { return _where; }

        llvm::Value* default_value() const { return abstract_pointer(); }

        llvm::Value *_what;
        llvm::Instruction *_where;
    };

    struct argument
    {
        llvm::Value * value;
        bool liftable;

        template< typename stream >
        friend auto operator<<( stream &s, argument a ) -> decltype( s << "" )
        {
            auto lift = a.liftable ? "lift" : "concrete";
            return s << sc::fmt::llvm_name( a.value ) << ":" << lift;
        }
    };

    static constexpr bool lift = true;
    static constexpr bool concrete = !lift;

    using args_t = std::vector< argument >;

    struct melt : base
    {
        std::string name() const { return "melt"; }
        args_t arguments() const
        {
            auto load = llvm::cast< llvm::LoadInst >( _what );
            return { { load->getPointerOperand(), concrete } };
        }
    };

    struct freeze : base
    {
        std::string name() const { return "freeze"; }
        args_t arguments() const
        {
            auto store = llvm::cast< llvm::StoreInst >( _what );
            return {
                { store->getValueOperand(), lift },
                { store->getPointerOperand(), concrete }
            };
        }
    };

    struct binary : base
    {
        std::string name() const { return "bin"; }
        args_t arguments() const
        {
            auto bin = llvm::cast< llvm::BinaryOperator >( _what );
            return {
                { bin->getOperand( 0 ), lift },
                { bin->getOperand( 1 ), lift },
            };
        }
    };

    struct alloc : base
    {
        std::string name() const { return "alloca"; }
        args_t arguments() const { return {}; }
    };

    struct store : base
    {
        std::string name() const { return "store"; }
        args_t arguments() const { return {}; }
    };

    struct load : base
    {
        std::string name() const { return "load"; }
        args_t arguments() const { return {}; }
    };

    using operation = std::variant<
        melt, freeze,
        binary,
        alloc, store, load >;

    namespace detail
    {
        static auto invoke = [] (auto f) { return [=] ( auto a ) { return std::visit(f, a); }; };
    } // namespace detail

    static auto location  = detail::invoke( [] (auto o) { return o.location(); } );
    static auto name      = detail::invoke( [] (const auto &o) { return o.name(); } );
    static auto arguments = detail::invoke( [] (const auto &o) { return o.arguments(); } );
    static auto default_value = detail::invoke( [] (const auto &o) { return o.default_value(); } );

    namespace sv = sc::views;

    inline auto unique_name_suffix(const operation &o)
    {
        auto values = sv::map([] (const auto &o_) { return o_.value; });
        auto format = sv::map([] (auto t) { return sc::fmt::type(t); });

        std::stringstream suff;
        auto args = arguments(o);
        auto fmt = args | values | sv::types | format;
        std::copy(fmt.begin(), fmt.end(), std::experimental::make_ostream_joiner(suff, "."));
        return suff.str();
    }

    template< typename stream >
    auto operator<<( stream &s, const args_t &a ) -> decltype( s << "" )
    {
        s << '[';
        std::copy(a.begin(), a.end(), std::experimental::make_ostream_joiner(s, ", "));
        s << "]";
        return s;
    }

} // namespace lart::op
