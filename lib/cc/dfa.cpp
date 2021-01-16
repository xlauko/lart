/*
 * (c) 2020, 2021 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <cc/dfa.hpp>

#include <cc/logger.hpp>

namespace lart::detail
{
    void dataflow_analysis::push( edge &&e ) noexcept
    {
        spdlog::info( "push {}", e );
        worklist.push( e );
    }

    void dataflow_analysis::push( llvm::Value *v ) noexcept
    {
        if ( llvm::isa< llvm::ConstantData >( v ) )
            return; //ignore

        if ( util::is_one_of< llvm::Instruction, llvm::Argument >( v ) )
            prep.run( sc::get_function( v ) );

        for ( auto && e : edges( v ) )
            push( std::move( e ) );
    }

    auto dataflow_analysis::pop() noexcept -> edge
    {
        auto e = worklist.front();
        worklist.pop();
        spdlog::info( "pop {}", e );
        return e;
    }

    edges_t dataflow_analysis::edges( llvm::Value * ) const
    {
        return {};
    }

    void dataflow_analysis::run_from( const roots_map &roots )
    {
        // TODO obtain AA

        /*using lart::tag::enumerate;
        for ( auto r : enumerate( m, tag::abstract ) ) {
            types.emplace( r, type_from_meta( r ) );
            push( r );
        }

        for ( auto & fn : m ) {
            if ( lart::tag::has( &fn, tag::abstract ) )
                types.emplace( &fn, type_from_meta( &fn ) );
        }*/


        // abstract_meta( types, dfg, cr ).attach( m );

        for ( const auto&[k, v] : roots ) {
            push( k.getInstruction() );
        }

        while ( !worklist.empty() )
            process( pop() );
    }

    void dataflow_analysis::process( edge &&/*e*/ )
    {

    }

} // namespace lart::detail
