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
            : base( what, llvm::cast< llvm::Instruction >( what )
                                ->getNextNonDebugInstruction() )
        {}

        base( llvm::Value *what, llvm::Instruction *where )
            : _what( what ), _where( where )
        {}

        llvm::Value* value() { return _what; }
        llvm::Instruction* location() { return _where; }

        std::optional< llvm::Value* > default_value() const
        {
            return abstract_pointer();
        }

        llvm::Value *_what;
        llvm::Instruction *_where;
    };

    struct with_taints_base : base
    {
        using base::base;

        constexpr bool with_taints() const { return true; }
    };

    struct without_taints_base : base
    {
        using base::base;

        constexpr bool with_taints() const { return false; }
    };

    enum class argtype { test, lift, concrete };

    struct argument
    {
        llvm::Value * value;
        argtype type;

        template< typename stream >
        friend auto operator<<( stream &s, argument a ) -> decltype( s << "" )
        {
            auto lift = [a] {
                switch (a.type) {
                    case argtype::test: return "test";
                    case argtype::lift: return "lift";
                    case argtype::concrete: return "concrete";
                }
                __builtin_unreachable();
            } ();

            return s << sc::fmt::llvm_name( a.value ) << ":" << lift;
        }
    };

    using args_t = std::vector< argument >;

    struct melt : with_taints_base
    {
        using base = with_taints_base;
        using base::with_taints;

        std::string name() const { return "melt"; }
        std::string impl() const { return "__lart_melt"; }
        args_t arguments() const
        {
            auto load = llvm::cast< llvm::LoadInst >( _what );
            auto ptr = load->getPointerOperand();
            auto elem = ptr->getType()->getPointerElementType();
            return {
                { load, argtype::test },
                { ptr, argtype::concrete },
                { sc::i32( sc::bytes( elem ) ), argtype::concrete }
            };
        }
    };

    struct freeze : with_taints_base
    {
        std::string name() const { return "freeze"; }
        std::string impl() const { return "__lart_freeze"; }

        args_t arguments() const
        {
            auto store = llvm::cast< llvm::StoreInst >( _what );
            auto ptr = store->getPointerOperand();
            auto elem = ptr->getType()->getPointerElementType();
            return {
                { store->getValueOperand(), argtype::lift },
                { ptr, argtype::concrete },
                { sc::i32( sc::bytes( elem ) ), argtype::concrete }
            };
        }

        std::optional< llvm::Value* > default_value() const
        {
            return std::nullopt;
        }
    };

    struct binary : with_taints_base
    {
        std::string name() const { return "bin"; }
        std::string impl() const
        {
            auto bin = llvm::cast< llvm::BinaryOperator >( _what );
            return "__lamp_" + std::string(bin->getOpcodeName());
        }

        args_t arguments() const
        {
            auto bin = llvm::cast< llvm::BinaryOperator >( _what );
            return {
                { bin->getOperand( 0 ), argtype::lift },
                { bin->getOperand( 1 ), argtype::lift },
            };
        }
    };

    struct cast : with_taints_base
    {
        std::string name() const { return "cast"; }
        std::string impl() const
        {
            auto c = llvm::cast< llvm::CastInst >( _what );
            return "__lamp_" + std::string(c->getOpcodeName());
        }

        args_t arguments() const
        {
            auto c = llvm::cast< llvm::CastInst >( _what );
            auto dst = static_cast< uint8_t >( sc::bytes( c->getDestTy() ) );
            return {
                { c->getOperand( 0 ), argtype::lift },
                { sc::i8( dst ), argtype::concrete }
            };
        }
    };

    struct cmp : with_taints_base
    {
        std::string name() const { return "cmp"; }
        std::string impl() const
        {
            auto c = llvm::cast< llvm::CmpInst >( _what );
            auto pred = llvm::CmpInst::getPredicateName( c->getPredicate() );
            return "__lamp_" + std::string( pred );
        }

        args_t arguments() const
        {
            auto c = llvm::cast< llvm::CmpInst >( _what );
            return {
                { c->getOperand( 0 ), argtype::lift },
                { c->getOperand( 1 ), argtype::lift },
            };
        }
    };

    struct alloc : with_taints_base
    {
        std::string name() const { return "alloca"; }
        std::string impl() const { return "__lamp_alloca"; }
        args_t arguments() const { return {}; }
    };

    struct store : with_taints_base
    {
        std::string name() const { return "store"; }
        std::string impl() const { return "__lamp_store"; }
        args_t arguments() const { return {}; }
    };

    struct load : with_taints_base
    {
        std::string name() const { return "load"; }
        std::string impl() const { return "__lamp_load"; }
        args_t arguments() const { return {}; }
    };

    struct stash_base
    {
        inline static const std::string name = "stash";
        inline static const std::string impl = "__lart_stash";
    };

    struct stash : without_taints_base
    {
        std::string name() const { return stash_base::name; }
        std::string impl() const { return stash_base::impl; }
        args_t arguments() const { return {}; }
    };


    struct unstash_base
    {
        inline static const std::string name = "unstash";
        inline static const std::string impl= "__lart_unstash";
    };

    struct unstash : without_taints_base, unstash_base
    {
        std::string name() const { return unstash_base::name; }
        std::string impl() const { return unstash_base::impl; }
        args_t arguments() const { return {}; }
    };

    using operation = std::variant<
        melt, freeze,
        binary, cast, cmp,
        alloc, store, load,
        stash, unstash >;

    namespace detail
    {
        static auto invoke = [] (auto f) { return [=] ( auto a ) { return std::visit(f, a); }; };
    } // namespace detail

    static auto value     = detail::invoke( [] (auto o) { return o.value(); } );
    static auto location  = detail::invoke( [] (auto o) { return o.location(); } );
    static auto name      = detail::invoke( [] (const auto &o) { return o.name(); } );
    static auto impl      = detail::invoke( [] (const auto &o) { return o.impl(); } );
    static auto arguments = detail::invoke( [] (const auto &o) { return o.arguments(); } );
    static auto default_value = detail::invoke( [] (const auto &o) { return o.default_value(); } );
    static auto with_taints = detail::invoke( [] (const auto &o) { return o.with_taints(); } );

    inline bool returns_value( const operation &o )
    {
        return default_value(o).has_value();
    }

    namespace sv = sc::views;

    template< typename T > using generator = cppcoro::generator< T >;

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

    inline generator< llvm::Value* > duplicated_arguments(const operation &op)
    {
        for ( auto arg : op::arguments(op) ) {
            if ( arg.type == argtype::lift ) {
                co_yield arg.value;
                co_yield op::abstract_pointer();
            } else {
                co_yield arg.value;
            }
        }
    }

    template< typename stream >
    auto operator<<( stream &s, const args_t &a ) -> decltype( s << "" )
    {
        s << '[';
        std::copy(a.begin(), a.end(), std::experimental::make_ostream_joiner(s, ", "));
        s << "]";
        return s;
    }

    inline llvm::Function* function( const operation &op, llvm::Module *module
                                    , const std::vector< llvm::Type * > &args
                                    , const std::string &intr_name )
    {
        auto out = op::default_value(op);
        auto rty = out.has_value() ? out.value()->getType() : sc::void_t();
        auto fty = llvm::FunctionType::get( rty, args, false );
        auto fn = llvm::cast< llvm::Function >(
            module->getOrInsertFunction( intr_name, fty ).getCallee()
        );
        fn->addFnAttr( llvm::Attribute::NoUnwind );
        return fn;
    }

    inline llvm::CallInst* make_call( const operation &op
                                         , const std::vector< llvm::Value * > &args
                                         , const std::string &intr_name )
    {
        llvm::IRBuilder<> irb( op::location(op) );
        auto module = irb.GetInsertBlock()->getModule();
        auto arg_types = sv::freeze( args | sv::types );
        return irb.CreateCall( function(op, module, arg_types, intr_name), args );
    }

} // namespace lart::op
