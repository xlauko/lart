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

#include <cc/logger.hpp>

#include <sc/query.hpp>
#include <sc/format.hpp>
#include <sc/constant.hpp>

#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/ErrorHandling.h>

#include <variant>
#include <experimental/iterator>

#include <sc/warnings.hpp>

#include <sc/generator.hpp>

namespace lart::op
{
    struct abstract_pointer_default {};
    struct concrete_value { sc::value value; };
    struct concrete_argument_default { unsigned index; };

    inline auto abstract_pointer()
    {
        return sc::null( sc::i8p() );
    }

    using default_wrapper = std::variant<
        abstract_pointer_default, concrete_value, concrete_argument_default
    >;

    static inline llvm::Instruction* get_insertion_point( sc::value what ) {
        if ( auto arg = llvm::dyn_cast< llvm::Argument >( what ) ) {
            return arg->getParent()->getEntryBlock().getFirstNonPHIOrDbgOrLifetime();
        }

        return llvm::cast< llvm::Instruction >( what )->getNextNonDebugInstruction();
    }

    template< bool _emit_test_taint >
    struct base
    {
        base( sc::value what )
            : base( what, get_insertion_point( what ) )
        {}

        base( sc::value what, llvm::Instruction *where )
            : _what( what ), _where( where )
        {}

        sc::value value() { return _what; }
        llvm::Instruction* location() { return _where; }

        std::optional< default_wrapper > default_value() const
        {
            return abstract_pointer_default();
        }

        sc::value _what;
        llvm::Instruction *_where;

        std::optional< sc::value > replaces;

        constexpr bool emit_test_taint() const { return _emit_test_taint; }

        constexpr bool with_taints() const { return false; }
    };

    template< bool emit_test_taint >
    struct with_taints_base : base< emit_test_taint >
    {
        using base< emit_test_taint >::base;

        constexpr bool with_taints() const { return true; }
    };

    struct with_test_taints_base : with_taints_base< true > {
        using base = with_taints_base< true >;
        using base::base;
    };

    struct without_taints_base : base< false /* do not emit test taint */ >
    {
        using base::base;
    };

    enum class argtype { test, with_taint, concrete, abstract, unpack };

    static inline std::string to_string( argtype type ) {
        switch (type) {
            case argtype::test: return "test";
            case argtype::with_taint: return "tainted";
            case argtype::concrete: return "concrete";
            case argtype::abstract: return "abstract";
            case argtype::unpack: return "unpack";
        }

        llvm_unreachable( "unknown arg type" );
    }

    struct argument
    {
        llvm::Value *value;
        argtype type;

        static inline constexpr bool is_abstract(const argument & arg)
        {
            return arg.type == argtype::abstract;
        }
    };

    static inline std::string to_string( argument a ) {
        return sc::fmt::llvm_name( a.value ) + ":" + to_string(a.type);
    }

    template< typename stream >
    auto operator<<( stream &s, argument a ) -> decltype( s << "" )
    {
        return s << to_string( a );
    }


    using args_t = std::vector< argument >;

    struct melt : with_test_taints_base
    {
        using base = with_test_taints_base;
        using base::with_taints;

        std::string name() const { return "melt"; }
        std::string impl() const { return "__lamp_melt"; }
        args_t arguments() const
        {
            auto load = llvm::cast< llvm::LoadInst >( _what );
            auto ptr  = load->getPointerOperand();
            auto elem = ptr->getType()->getPointerElementType();
            auto dl   = sc::data_layout( load );
            return {
                { load, argtype::test },
                { ptr, argtype::concrete },
                { sc::i64( sc::bytes( elem, dl ) ), argtype::concrete }
            };
        }
    };

    struct freeze : with_taints_base< false /* do not emit test taint */ >
    {
        std::string name() const { return "freeze"; }
        std::string impl() const { return "__lamp_freeze"; }

        args_t arguments() const
        {
            auto store = llvm::cast< llvm::StoreInst >( _what );
            auto ptr = store->getPointerOperand();
            auto elem = ptr->getType()->getPointerElementType();
            auto dl   = sc::data_layout( store );
            return {
                { store->getValueOperand(), argtype::with_taint },
                { ptr, argtype::concrete },
                { sc::i64( sc::bytes( elem, dl ) ), argtype::concrete }
            };
        }

        std::optional< default_wrapper > default_value() const
        {
            return std::nullopt;
        }
    };

    struct binary : with_test_taints_base
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
                { bin->getOperand( 0 ), argtype::with_taint },
                { bin->getOperand( 1 ), argtype::with_taint },
            };
        }
    };

    struct cast : with_test_taints_base
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
                { c->getOperand( 0 ), argtype::with_taint },
                { sc::i8( dst ), argtype::concrete }
            };
        }
    };

    struct cmp : with_test_taints_base
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
                { c->getOperand( 0 ), argtype::with_taint },
                { c->getOperand( 1 ), argtype::with_taint },
            };
        }
    };

    struct assume : with_test_taints_base
    {
        assume( llvm::Value *what, llvm::Instruction *where, llvm::Constant *exp )
            : with_test_taints_base( what, where ), expected( exp )
        {}

        std::string name() const { return "assume"; }
        std::string impl() const { return "__lamp_assume"; }

        args_t arguments() const
        {
            return {
                { _what, argtype::with_taint },
                { expected, argtype::concrete },
            };
        }

        std::optional< default_wrapper > default_value() const
        {
            return std::nullopt;
        }

        llvm::Constant *expected;
    };

    struct alloc : with_test_taints_base
    {
        std::string name() const { return "alloca"; }
        std::string impl() const { return "__lamp_alloca"; }
        args_t arguments() const { return {}; }
    };

    struct store : with_test_taints_base
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
                { ptr, argtype::with_taint },
                { val, argtype::with_taint },
                { sc::i8( uint8_t( sc::bytes( elem, dl ) ) ), argtype::concrete }
            };
        }

        std::optional< default_wrapper > default_value() const
        {
            return std::nullopt;
        }
    };

    struct load : with_test_taints_base
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
                { ptr, argtype::with_taint },
                { sc::i8( static_cast< uint8_t >( sc::bytes( elem, dl ) ) )
                , argtype::concrete }
            };
        }
    };

    struct stash : with_taints_base< false /* do not emit test taint */ >
    {
        using base = with_taints_base< false >;

        stash( llvm::Value *what, llvm::Instruction *call )
            : base( what, call )
        {}

        std::string name() const
        {
            return "stash." + sc::fmt::llvm_to_string(_what->getType());
        }

        std::string impl() const { return "__lart_stash"; }

        args_t arguments() const
        {
            return {{ _what, argtype::with_taint }};
        }

        std::optional< default_wrapper > default_value() const
        {
            return std::nullopt;
        }
    };

    struct unstash : without_taints_base
    {
        using base = without_taints_base;
        using base::base;

        std::string name() const { return "unstash"; }
        std::string impl() const { return  "__lart_unstash"; }
        args_t arguments() const { return {}; }
    };

    struct unstash_taint : without_taints_base
    {
        using base = without_taints_base;
        using base::base;

        std::string name() const { return "unstash_taint"; }
        std::string impl() const { return  "__lart_unstash_taint"; }
        args_t arguments() const { return {}; }

        std::optional< default_wrapper > default_value() const
        {
            return concrete_value{ sc::i1(false) };
        }
    };

    struct tobool : with_test_taints_base
    {
        explicit tobool( llvm::BranchInst *br )
            : with_test_taints_base( br, br )
        {
            replaces = br->getCondition();
        }

        std::string name() const { return "tobool"; }
        std::string impl() const { return "__lamp_to_bool"; }

        args_t arguments() const
        {
            auto br = llvm::cast< llvm::BranchInst >( _what );
            return { { br->getCondition(), argtype::with_taint } };
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

    struct phi : without_taints_base {
        explicit phi(llvm::PHINode *node) : without_taints_base(node) {}

        std::string name() const { return "phi"; }
        std::string impl() const { llvm_unreachable( "invalid op" ); }
        args_t arguments() const { llvm_unreachable( "invalid op" ); }

        std::optional< default_wrapper > default_value() const
        {
            return concrete_argument_default{ 1 };
        }
    };

    struct dump : without_taints_base
    {
        using base = without_taints_base;
        using base::base;

        std::string name() const { return "dump"; }
        std::string impl() const { return  "__lamp_dump"; }
        args_t arguments() const
        {
            return {{ _what, argtype::concrete }};
        }

        std::optional< default_wrapper > default_value() const
        {
            return std::nullopt;
        }
    };

    using operation = std::variant<
        melt, freeze,
        binary, cast, cmp,
        tobool, assume,
        alloc, store, load,
        stash, unstash, unstash_taint, identity, phi,
        dump
    >;

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
    static auto emit_test_taint = detail::invoke( [] (const auto &o) { return o.emit_test_taint(); } );
    static auto replaces = detail::invoke( [] (const auto &o) { return o.replaces; } );

    inline bool with_abstract_arg( const operation &o )
    {
        return sc::query::query(arguments(o)).any( argument::is_abstract );
    }

    inline bool returns_value( const operation &o )
    {
        return default_value(o).has_value();
    }

    inline auto extract_default( default_wrapper def, const std::vector< sc::value > &args )
        -> sc::value
    {
        if ( std::holds_alternative< abstract_pointer_default >( def ) )
            return abstract_pointer();
        if ( std::holds_alternative< concrete_value >( def ) )
            return std::get< concrete_value >( def ).value;
        auto idx = std::get< concrete_argument_default >( def ).index;
        return args[ idx ];
    }

    inline auto extract_default( default_wrapper def, const std::vector< sc::type > &types )
        -> sc::type
    {
        if ( std::holds_alternative< abstract_pointer_default >( def ) )
            return abstract_pointer()->getType();
        if ( std::holds_alternative< concrete_value >( def ) )
            return std::get< concrete_value >( def ).value->getType();
        auto idx = std::get< concrete_argument_default >( def ).index;
        return types[ idx ];
    }

    inline auto extract_return_type( const operation &op, const std::vector< llvm::Type* > &types )
        -> llvm::Type*
    {
        auto out = op::default_value(op);
        return out.has_value() ? extract_default( out.value(), types ) : sc::void_t();
    }

    inline auto unique_name_suffix(const operation &o)
    {
        std::stringstream suff;
        auto args = arguments(o);
        auto get_value = [] (auto op) { return op.value; };
        auto fmt = sc::query::query( args ).map( get_value ).map( sc::query::type ).map( sc::fmt::type );

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
        auto mod = irb.GetInsertBlock()->getModule();
        auto arg_types = sc::query::types( args ).freeze();
        auto rty = extract_return_type( op, arg_types );

        spdlog::debug("[cc] make call: {} :: {} -> {}", intr_name, sc::fmt::llvm_to_string(arg_types), sc::fmt::llvm_to_string(rty));
        return irb.CreateCall( function(mod, rty, arg_types, intr_name), args );
    }

} // namespace lart::op
