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

#include <cc/lifter.hpp>
#include <cc/util.hpp>

#include <sc/query.hpp>
#include <sc/generator.hpp>
#include <sc/builder.hpp>

#include <ranges>

#include <numeric>
#include <algorithm>

namespace lart
{
    namespace arg
    {
        struct with_taint
        {
            llvm::Value *taint;
            llvm::Value *concrete;
            llvm::Value *abstract;
        };

        struct without_taint_concrete
        {
            llvm::Value *value;
        };

        struct without_taint_abstract
        {
            llvm::Value *concrete;
            llvm::Value *abstract;
        };

    } // namespace arg

    using argument = std::variant< arg::with_taint, arg::without_taint_concrete, arg::without_taint_abstract >;

    namespace detail
    {
        sc::generator< argument > arguments( const lifter &lif )
        {
            auto *f = lif.function();
            unsigned i = 0;
            for ( auto arg : op::arguments(lif.op) ) {
                switch ( arg.type ) {
                case op::argtype::test:
                    break; // ignore test arguments in lifter
                case op::argtype::with_taint:
                    co_yield arg::with_taint{ f->getArg(i), f->getArg(i + 1), f->getArg(i + 2) };
                    i += 3;
                    break;
                case op::argtype::concrete:
                    co_yield arg::without_taint_concrete{ f->getArg(i) };
                    i += 1;
                    break;
                case op::argtype::abstract:
                    co_yield arg::without_taint_abstract{ f->getArg(i), f->getArg(i + 1) };
                    i += 3;
                    break;
                }
            }
        }

        static auto final = [] ( const argument &arg ) -> llvm::Value*
        {
            return std::visit( util::overloaded {
                [] ( arg::with_taint a ) { return a.abstract; },
                [] ( arg::without_taint_concrete a ) { return a.value; },
                [] ( arg::without_taint_abstract a ) { return a.abstract; }
            }, arg );
        };

        static auto with_taint = [] ( const argument &arg ) -> bool
        {
            return std::holds_alternative< arg::with_taint >( arg );
        };

        auto count_taints( const std::vector< argument > &args )
        {
            return std::count_if( args.begin(), args.end(), with_taint );
        }

        auto final_args( llvm::BasicBlock *where
                       , const std::vector< argument > &args
                       , const auto &types )
        {
            size_t count = 0;
            return sc::query::query( args ).map( [&] (const auto &a) {
                auto value = final( a );
                auto dst = types[ count++ ];
                if  ( value->getType() != dst ) {
                    llvm::IRBuilder<> irb( where );
                    return irb.CreatePointerCast( value, dst );
                }
                return value;
            }).freeze();
        }

    } // namespace detail

    std::string lifter::name() const
    {
        return "lart.lifter." + op::name(op) + "." + op::unique_name_suffix(op);
    }

    llvm::Function* lifter::function() const
    {
        if ( _function )
            return _function;

        auto aptr = op::abstract_pointer()->getType();

        std::vector< llvm::Type * > args;
        for ( auto arg : op::arguments(op) ) {
            switch ( arg.type ) {
            case op::argtype::test:
                break; // ignore test arguments in lifters
            case op::argtype::with_taint:
                args.push_back(sc::i1()); // dummy false
                args.push_back(arg.value->getType());
                args.push_back(aptr);
                break;
            case op::argtype::concrete:
                args.push_back(arg.value->getType());
                break;
            case op::argtype::abstract:
                args.push_back(arg.value->getType());
                args.push_back(aptr);
                break;
            }
        }

        auto rty = extract_return_type( op, args );
        _function = op::function( &module, rty, args, name() );
        return _function;
    }

    void lifter::generate() const
    {
        assert( function()->empty() );

        auto bld = sc::stack_builder()
                 | sc::action::function{ function() }
                 | sc::action::create_block{ "entry" };

        std::vector< argument > args;
        for (auto arg : detail::arguments( *this )) {
            args.push_back(arg);
        }

        auto wrap = [&] ( auto val ) {
            auto name = "__lamp_wrap_" + [&] {
                auto ty = val->getType();
                return ty->isPointerTy() ? "ptr" : sc::fmt::llvm_name( ty );
            } ();
            return module.getFunction( name );
        };

        using args_t = std::vector< llvm::Value* >;
        auto lift = [&] ( auto &arg, unsigned pos ) {
            std::vector< sc::phi_edge > edges;

            std::string entry_block = bld.current_block_name();
            std::string lift_block  = "lift." + std::to_string(pos);
            std::string merge_block = "merge." + std::to_string(pos);
            if ( auto a = std::get_if< arg::with_taint >( &arg ) ) {
                bld = std::move(bld)
                    | sc::action::create_block{ lift_block }
                    | sc::action::create_block{ merge_block }
                    | sc::action::set_block{ entry_block }
                    /* entry block to lift section */
                    | sc::action::inspect( [&]( auto *builder ) {
                        edges.push_back({ a->abstract, builder->current_block });
                    });
                auto mbb = bld.block( merge_block );
                auto lbb = bld.block( lift_block );

                args_t wrap_args{ a->concrete };

                bld = std::move(bld)
                    | sc::action::set_block{ entry_block }
                    | sc::action::condbr( a->taint, mbb, lbb )
                    | sc::action::set_block{ lift_block }
                    /* lift block */
                    | sc::action::call( wrap( a->concrete ), wrap_args )
                    | sc::action::inspect( [&]( auto *builder ) {
                        auto wrapped = builder->stack.back();
                        edges.push_back({ wrapped, builder->current_block });
                    })
                    | sc::action::branch{ mbb }
                    | sc::action::set_block{ merge_block };
                bld = std::move(bld)
                    /* merge block */
                    | sc::action::phi{ edges };

                // set abstract value to be merge of lifted and argument value
                a->abstract = bld.stack.back();
            }
        };

        // for trivial lifter there is no need to lift anything when
        // only one argument can be tainted. If the test.taint is triggered
        // we know, that the single argument is abstract.
        if ( detail::count_taints( args ) > 1 ) {
            unsigned pos = 1;
            for ( auto &arg : args )
                lift( arg, pos++ );
        }

        auto impl = module.getFunction( op::impl(op) );
        if (!impl) {
            spdlog::error("missing: {}",op::impl(op));
        }

        auto types = impl->getFunctionType()->params();
        auto final_args = detail::final_args( bld.current_block, args, types );
        std::move(bld) | sc::action::call{ impl, std::span(final_args) }
                       | sc::action::ret();
    }

} // namespace lart
