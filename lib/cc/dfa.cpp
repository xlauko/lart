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

#include <cc/util.hpp>
#include <cc/logger.hpp>

#include <llvm/IR/IntrinsicInst.h>

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

    edges_t dataflow_analysis::edges( llvm::Value *v ) const
    {
        edges_t edges;

        auto use = [&] ( llvm::Value * f, llvm::Value * t ) {
            auto e = induced_edges( f, t );
            std::move( e.begin(), e.end(), std::back_inserter( edges ) );
        };

        // creates edge between operand and function arguments
        auto arguse = [&] ( auto /*operand*/, llvm::CallSite /*call*/ ) {
            /*for ( auto fn : cr.resolve_call( call ) ) {
                if ( auto afn = abstract_function( fn ) ) {
                    if ( lart::tag::has( afn, tag::abstract ) )
                        use( call.getInstruction(), afn );
                // FIXME transform function even if abstract version exists (perform copy)
                } else if ( !lart::tag::function::ignore_call( fn ) ) {
                    bool matched = false;
                    for ( auto & op : call.args() ) {
                        if ( op.get() == operand ) {
                            ASSERT( !( fn->isVarArg() && op.getOperandNo() >= fn->arg_size() ),
                                    "DFA cannot handle abstract varargs" );
                            use( operand, argument( fn, op.getOperandNo() ) );
                            matched = true;
                        }
                    }
                    if ( !matched )
                        UNREACHABLE( "mismatch of call and operand" );
                }
            }*/
        };

        auto calluse = [&] ( auto operand, llvm::CallSite call ) {
            if ( operand == call.getCalledValue() )
                use( v, call.getInstruction() ); // use of function return value
            else
                arguse( operand, call );
        };

        auto keep_intrinsic = [] ( auto id )
        {
            switch ( id ) {
                case llvm::Intrinsic::lifetime_start:
                case llvm::Intrinsic::lifetime_end:
                    return true;
                default:
                    return false;
            }
        };

        auto forward = [ & ]( llvm::Value * user ) {
            sc::llvmcase( user,
                [&] ( llvm::IntrinsicInst * i ) {
                    if ( keep_intrinsic( i->getIntrinsicID() ) )
                        return;
                    // FIXME
                    // TRACE( "ignoring intrinsic: ", to_string( i ) );
                },
                [&] ( llvm::CallInst     * call ) { calluse( v, call ); },
                [&] ( llvm::InvokeInst   * call ) { calluse( v, call ); },
                [&] ( llvm::Instruction  * inst ) { use( v, inst ); },
                [&] ( llvm::ConstantExpr * expr ) { use( v, expr ); },
                [&] ( llvm::GlobalVariable * gv ) { use( v, gv ); },
                []  ( llvm::Value */*v*/ ) {
                    //UNREACHABLE( "unsupported edge", to_string( v ) );
                }
            );
        };

        /*auto in_abstractable_function = [] ( auto value ) -> bool {
            if ( llvm::isa< llvm::GlobalValue >( value ) )
                return false;
            if ( llvm::isa< llvm::ConstantExpr >( value ) )
                return false;
            return abstract_function( sc::get_function( value ) );
        };*/

        auto users = [&] ( auto node ) -> std::vector< llvm::Value * > {
            /*if ( auto aml = dfg.gv_to_aml( node ) )
                return query::query( aml->succs )
                    .filter( std::not_fn( in_abstractable_function ) )
                    .filter( std::not_fn( in_lava ) )
                    .filter( std::not_fn( in_lamp ) )
                    .freeze();*/
            return { node->users().begin(), node->users().end() };
        };

        for ( auto u : users( v ) )
            forward( u );

        return edges;
    }

    edges_t dataflow_analysis::induced_edges( llvm::Value * lhs, llvm::Value * rhs ) const
    {
        using type = edge::type;

        auto unif_edge =  [] ( auto l, auto r ) { return edge{ l, r, type::uniform }; };
        auto load_edge =  [] ( auto l, auto r ) { return edge{ l, r, type::load }; };
        //auto store_edge = [] ( auto l, auto r ) { return edge{ l, r, type::store }; };

        edges_t edges;

        auto push = [&] ( auto e ) { edges.push_back( e ); };

        sc::llvmcase( rhs,
            [&] ( llvm::Argument * )          { push( unif_edge( lhs, rhs ) ); },
            [&] ( llvm::AllocaInst * )        { push( unif_edge( lhs, rhs ) ); },
            [&] ( llvm::CmpInst * )           { push( unif_edge( lhs, rhs ) ); },
            [&] ( llvm::BinaryOperator * )    { push( unif_edge( lhs, rhs ) ); },
            [&] ( llvm::GetElementPtrInst * ) { push( unif_edge( lhs, rhs ) ); },
            [&] ( llvm::PHINode * )           { push( unif_edge( lhs, rhs ) ); },
            [&] ( llvm::CastInst * )          { push( unif_edge( lhs, rhs ) ); },
            [&] ( llvm::CallInst * )          { push( unif_edge( lhs, rhs ) ); },
            [&] ( llvm::InvokeInst * )        { push( unif_edge( lhs, rhs ) ); },
            [&] ( llvm::ConstantExpr * )      { push( unif_edge( lhs, rhs ) ); },
            [&] ( llvm::GlobalVariable * )    { push( unif_edge( lhs, rhs ) ); },
            [&] ( llvm::LoadInst * )          { push( load_edge( lhs, rhs ) ); },
            [&] ( llvm::StoreInst * )
            {
                /*if ( lhs == s->getValueOperand() )
                {
                    auto amls = dfg.get( s->getPointerOperand() );
                    for ( auto aml : amls )
                        push( store_edge( lhs, aml->gv ) );
                    stores.insert( s );
                }*/
            },
            [&] ( llvm::ReturnInst * r )
            {
                // ASSERT( lhs == r->getReturnValue() );
                push( unif_edge( r->getReturnValue(), sc::get_function( rhs ) ) );
            },
            [&] ( llvm::Function * )
            {
                // ASSERT( lart::tag::has( fn, tag::abstract ) );
                push( unif_edge( rhs, lhs ) );
            },
            [] ( llvm::BranchInst *)    { /* ignore */ },
            [] ( llvm::ConstantData * ) { /* ignore */ },
            [] ( llvm::Value * ) { /*UNREACHABLE( "unsupported edge", to_string( v ) );*/ }
        );

        return edges;
    }

    void dataflow_analysis::run_from( const roots_map &roots )
    {
        // TODO obtain AA

        /* for ( auto & fn : m ) {
            if ( lart::tag::has( &fn, tag::abstract ) )
                types.emplace( &fn, type_from_meta( &fn ) );
        }*/

        // abstract_meta( types, dfg, cr ).attach( m );

        for ( const auto&[call, kind] : roots ) {
            auto inst = call.getInstruction();
            types.add( inst, kind );
            push( inst );
        }

        while ( !worklist.empty() )
            process( pop() );
    }

    void dataflow_analysis::process( edge &&/*e*/ )
    {

    }

} // namespace lart::detail
