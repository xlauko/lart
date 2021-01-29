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

#include <sc/ranges.hpp>
#include <sc/builder.hpp>

#include <numeric>

namespace lart
{
    namespace sv = sc::views;

    template< typename T > using generator = cppcoro::generator< T >;

    namespace arg
    {
        struct with_taint
        {
            llvm::Argument *taint;
            llvm::Argument *concrete;
            llvm::Argument *abstract;
        };

        struct without_taint
        {
            llvm::Argument *value;
        };

    } // namespace arg

    using argument = std::variant< arg::with_taint, arg::without_taint >;

    namespace detail
    {
        generator< argument > arguments( const lifter &lif )
        {
            auto f = lif.function();

            unsigned i = 0;
            for ( auto arg : op::arguments(lif.op) ) {
                if ( arg.liftable ) {
                    co_yield arg::with_taint{ f->getArg(i), f->getArg(i + 1), f->getArg(i + 2) };
                    i += 3;
                } else {
                    co_yield arg::without_taint{ f->getArg(i) };
                    i++;
                }
            }
        }

        static auto final = [] ( const argument &arg ) -> llvm::Value*
        {
            return std::visit( util::overloaded {
                [] ( arg::with_taint a ) { return a.abstract; },
                [] ( arg::without_taint a ) { return a.value; },
            }, arg );
        };

        static auto with_taint = [] ( const argument &arg ) -> bool
        {
            return std::holds_alternative< arg::with_taint >( arg );
        };

        auto count_taints( const std::vector< argument > &args )
        {
            return std::ranges::count_if( args, with_taint );
        }

        auto final_args( const std::vector< argument > &args )
        {
            std::vector< llvm::Value* > fargs;
            std::ranges::transform( args, std::back_inserter(fargs), final );
            return fargs;
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
            if ( arg.liftable ) {
                args.push_back(sc::i1()); // dummy false
                args.push_back(arg.value->getType());
                args.push_back(aptr);
            } else {
                args.push_back(arg.value->getType());
            }
        }

        _function = op::intrinsic( op, &module, args, name() );
        return _function;
    }

    void lifter::generate() const
    {
        assert( function()->empty() );

        auto bld = sc::stack_builder()
                 | sc::action::function( function() )
                 | sc::action::create_block( "entry" );

        auto args = sv::freeze( detail::arguments( *this ) );

        auto wrap = [&] ( auto val ) {
            auto name = "__lamp_wrap_" + sc::fmt::llvm_name( val->getType() );
            return module.getFunction( name );
        };

        llvm::Value *lifted = nullptr;
        using args_t = std::vector< llvm::Value* >;
        auto lift = [&] ( auto arg, unsigned pos ) {
            if ( auto a = std::get_if< arg::with_taint >( &arg ) ) {
                bld = bld
                    | sc::action::create_block( "lift." + std::to_string(pos) )
                    | sc::action::create_block( "merge." + std::to_string(pos) )
                    | sc::action::advance_block( -2 )
                    /* entry block to lift section */
                    | sc::action::call( wrap( a->concrete ), args_t{ a->concrete } )
                    | sc::action::inspect( [&] { lifted = bld.stack.back(); } )
                    | sc::action::condbr( a->taint )
                    | sc::action::advance_block( 1 )
                    /* lift block */
                    | sc::action::branch()
                    | sc::action::advance_block( 1 );
                    /* merge block */
                    // TODO create phi of lifted and argument value
                // TODO update abstract argument
            }
        };

        // for trivial lifter there is no need to lift anything when
        // only one argument can be tainted. If the test.taint is triggered
        // we know, that the single argument is abstract.
        if ( detail::count_taints( args ) > 1 ) {
            unsigned pos = 1;
            for ( auto arg : args )
                lift( arg, pos++ );
        }

        auto impl = module.getFunction( op::impl(op) );
        bld | sc::action::call( impl, detail::final_args( args ) ) | sc::action::ret();

        function()->dump();
    }

} // namespace lart
