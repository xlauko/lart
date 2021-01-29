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
    } // namespace arg

    namespace detail
    {
        generator< unsigned > taint_indices( const lifter &lif )
        {
            unsigned pos = 0;
            for ( auto arg : op::arguments(lif.op) ) {
                if ( arg.liftable ) {
                    co_yield pos;
                    pos += 3;
                } else {
                    pos++;
                }
            }
        }

        generator< arg::with_taint > args_with_taints( const lifter &lif )
        {
            auto f = lif.function();
            for ( auto i : taint_indices(lif) )
                co_yield { f->getArg(i), f->getArg(i + 1), f->getArg(i + 2) };
        }

        generator< unsigned > argument_indices( const lifter &lif )
        {
            unsigned pos = 0;
            for ( auto arg : op::arguments(lif.op) ) {
                pos = arg.liftable ? pos + 2 : pos;
                co_yield pos;
                pos++;
            }
        }

        generator< llvm::Value* > arguments( const lifter &lif )
        {
            auto f = lif.function();
            for ( auto i : argument_indices(lif) )
                co_yield f->getArg( i );
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

        auto builder = sc::stack_builder()
                     | sc::action::function( function() )
                     | sc::action::create_block( "entry" );

        auto taints = sv::freeze( detail::args_with_taints( *this ) );
        auto args = sv::freeze( detail::arguments( *this ) );

        if ( taints.size() < 2 ) {
            // trivial lifter: no need to lift anything when only one argument
            // can be tainted. If the test.taint is triggered we know,
            // that the single argument is abstract.
            auto impl = module.getFunction( op::impl(op) );
            builder | sc::action::call( impl, args )
                    | sc::action::ret();
        }
    }

} // namespace lart
