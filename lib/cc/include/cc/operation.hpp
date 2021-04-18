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

//#include <cc/dfa.hpp>

#include <sc/ranges.hpp>
#include <sc/format.hpp>
#include <sc/constant.hpp>

#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/ErrorHandling.h>

#include <variant>
#include <experimental/iterator>

#include <cppcoro/generator.hpp>

namespace lart::op
{

    struct abstract_pointer_default {};
    struct concrete_argument_default { unsigned index; };

    inline auto abstract_pointer()
    {
        return sc::null( sc::i8p() );
    }

    using default_wrapper = std::variant<
        abstract_pointer_default, concrete_argument_default
    >;

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

        std::optional< default_wrapper > default_value() const
        {
            return abstract_pointer_default();
        }

        llvm::Value *_what;
        llvm::Instruction *_where;

        std::optional< llvm::Value* > replaces;
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
                    default: llvm_unreachable( "unknown arg type" );
                }
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
            auto ptr  = load->getPointerOperand();
            auto elem = ptr->getType()->getPointerElementType();
            auto dl   = sc::data_layout( load );
            return {
                { load, argtype::test },
                { ptr, argtype::concrete },
                { sc::i32( sc::bytes( elem, dl ) ), argtype::concrete }
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
            auto dl   = sc::data_layout( store );
            return {
                { store->getValueOperand(), argtype::lift },
                { ptr, argtype::concrete },
                { sc::i32( sc::bytes( elem, dl ) ), argtype::concrete }
            };
        }

        std::optional< default_wrapper > default_value() const
        {
            return std::nullopt;
        }
    };

    struct binary : with_taints_base
    {
        std::string name() const
        {
            auto bin = llvm::cast< llvm::BinaryOperator >( _what );
            return std::string( bin->getOpcodeName() );
        }
        std::string impl() const { return "__lamp_" + name(); }

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
        std::string name() const
        {
            auto c = llvm::cast< llvm::CastInst >( _what );
            return std::string(c->getOpcodeName());
        }
        std::string impl() const { return "__lamp_" + name(); }

        args_t arguments() const
        {
            auto c = llvm::cast< llvm::CastInst >( _what );
            auto dst = uint8_t( sc::bits( c ) );
            return {
                { c->getOperand( 0 ), argtype::lift },
                { sc::i8( dst ), argtype::concrete }
            };
        }
    };

    struct cmp : with_taints_base
    {
        std::string name() const
        {
            auto c = llvm::cast< llvm::CmpInst >( _what );
            auto pred = llvm::CmpInst::getPredicateName( c->getPredicate() );
            return std::string( pred );
        }
        std::string impl() const { return "__lamp_" + name(); }

        args_t arguments() const
        {
            auto c = llvm::cast< llvm::CmpInst >( _what );
            return {
                { c->getOperand( 0 ), argtype::lift },
                { c->getOperand( 1 ), argtype::lift },
            };
        }
    };

    struct assume : with_taints_base
    {
        assume( llvm::Value *what, llvm::Instruction *where, llvm::Constant *exp )
            : with_taints_base( what, where ), expected( exp )
        {}

        std::string name() const { return "assume"; }
        std::string impl() const { return "__lamp_assume"; }

        args_t arguments() const
        {
            return {
                { _what, argtype::lift },
                { expected, argtype::concrete },
            };
        }

        std::optional< default_wrapper > default_value() const
        {
            return std::nullopt;
        }

        llvm::Constant *expected;
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
        args_t arguments() const
        {
            auto what = llvm::cast< llvm::StoreInst >( _what );
            auto ptr = what->getPointerOperand();
            auto val = what->getValueOperand();
            auto elem = ptr->getType()->getPointerElementType();
            auto dl   = sc::data_layout( what );
            return {
                { ptr, argtype::lift },
                { val, argtype::lift },
                { sc::i8( uint8_t( sc::bytes( elem, dl ) ) ), argtype::concrete }
            };
        }
        
        std::optional< default_wrapper > default_value() const
        {
            return std::nullopt;
        }
    };

    struct load : with_taints_base
    {
        std::string name() const { return "load"; }
        std::string impl() const { return "__lamp_load"; }
        args_t arguments() const
        {
            auto what = llvm::cast< llvm::LoadInst >( _what );
            auto ptr = what->getPointerOperand();
            auto elem = ptr->getType()->getPointerElementType();
            auto dl   = sc::data_layout( what );
            return {
                { ptr, argtype::lift },
                { sc::i8( static_cast< uint8_t >( sc::bytes( elem, dl ) ) )
                , argtype::concrete }
            };
        }
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

    struct tobool : with_taints_base
    {
        explicit tobool( llvm::BranchInst *br )
            : with_taints_base( br, br )
        {
            replaces = br->getCondition();
        }

        std::string name() const { return "tobool"; }
        std::string impl() const { return "__lamp_to_bool"; }

        args_t arguments() const
        {
            auto br = llvm::cast< llvm::BranchInst >( _what );
            return { { br->getCondition(), argtype::lift } };
        }

        std::optional< default_wrapper > default_value() const
        {
            return concrete_argument_default{ 1 };
        }
    };

    struct identity : without_taints_base {
        std::string name() const { return "identity"; }
        std::string impl() const { llvm_unreachable( "invalid op" ); }
        args_t arguments() const { llvm_unreachable( "invalid op" ); }
    };

    using operation = std::variant<
        melt, freeze,
        binary, cast, cmp,
        tobool, assume,
        alloc, store, load,
        stash, unstash, identity >;

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
    static auto replaces = detail::invoke( [] (const auto &o) { return o.replaces; } );

    inline bool returns_value( const operation &o )
    {
        return default_value(o).has_value();
    }

    inline auto extract_default( default_wrapper def, const std::vector< llvm::Value* > &args )
        -> llvm::Value*
    {
        if ( std::holds_alternative< abstract_pointer_default >( def ) )
            return abstract_pointer();
        auto idx = std::get< concrete_argument_default >( def ).index;
        return args[ idx ];
    }

    inline auto extract_default( default_wrapper def, const std::vector< llvm::Type* > &types )
        -> llvm::Type*
    {
        if ( std::holds_alternative< abstract_pointer_default >( def ) )
            return abstract_pointer()->getType();
        auto idx = std::get< concrete_argument_default >( def ).index;
        return types[ idx ];
    }

    inline auto extract_return_type( const operation &op, const std::vector< llvm::Type* > &types )
        -> llvm::Type*
    {
        auto out = op::default_value(op);
        return out.has_value() ? extract_default( out.value(), types ) : sc::void_t();
    }

    namespace sv = sc::views;

    template< typename T > using generator = cppcoro::generator< T >;

    inline auto unique_name_suffix(const operation &o)
    {
        auto values = ranges::views::transform( [] (const auto &o_) { return o_.value; } );
        auto format = ranges::views::transform( [] (auto t) { return sc::fmt::type(t); } );

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

    inline llvm::Function* function( llvm::Module *module
                                   , llvm::Type *rty
                                   , const std::vector< llvm::Type * > &args
                                   , const std::string &intr_name )
    {
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
        auto arg_types = args | sv::types | ranges::to_vector;
        auto rty = extract_return_type( op, arg_types );
        return irb.CreateCall( function(module, rty, arg_types, intr_name), args );
    }

} // namespace lart::op
