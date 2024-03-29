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

#include <sc/format.hpp>

#include <cc/dfa.hpp>

#include <cc/alias.hpp>
#include <cc/util.hpp>
#include <cc/logger.hpp>

#include <llvm/Support/ErrorHandling.h>
#include <svf/SVF-FE/SVFIRBuilder.h>
#include <svf/SVF-FE/LLVMModule.h>
#include <svf/WPA/Andersen.h>


#include <llvm/IR/IntrinsicInst.h>

namespace lart::dfa {

    bool is_abstract( const dfa::types::type &type )
    {
        return static_cast< bool >( type.back().abstract );
    }

    bool is_abstract_pointer( const dfa::types::type &type )
    {
        return static_cast< bool >( type.back().pointer ) && is_abstract( type );
    }

} // namespace lart::dfa

namespace lart::dfa::detail
{
    static auto unif_edge =  [] ( auto l, auto r ) { return edge{ l, r, edge::type::uniform }; };
    static auto load_edge =  [] ( auto l, auto r ) { return edge{ l, r, edge::type::load }; };
    static auto store_edge = [] ( auto l, auto r ) { return edge{ l, r, edge::type::store }; };

    bool ignore_function( llvm::Function *fn )
    {
        if ( fn->getName().startswith( "__lamp" ) )
            return true;
        return false;
    }

    // returns lamp version of a function if it exists
    llvm::Function * abstract_function( llvm::Function *fn )
    {
        assert( fn->hasName() );
        auto mod  = fn->getParent();
        auto name = "__lamp_fn_" + fn->getName();
        return mod->getFunction( name.str() );
    }

    void dataflow_analysis::push( edge &&e ) noexcept
    {
        // spdlog::debug( "[dfa] push {}", e );
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
        // spdlog::debug( "[dfa] pop {}", e );
        return e;
    }

    sc::generator< llvm::Function * > dataflow_analysis::destinations( llvm::CallBase *call )
    {
        assert( !call->isIndirectCall() );
        co_yield call->getCalledFunction();
    }

    edges_t dataflow_analysis::edges( llvm::Value *v )
    {
        edges_t edges;

        auto forward_use = [&] ( llvm::Value *from, llvm::Value *to ) {
            auto e = induced_edges( from, to );
            std::move( e.begin(), e.end(), std::back_inserter( edges ) );
        };

        // creates edge between operand and function arguments
        auto arguse = [&] (const llvm::Use &use) {
            auto call = llvm::cast< llvm::CallBase >( use.getUser() );
            for ( auto fn : destinations(call) ) {
                if ( ignore_function(fn) )
                    continue;
                // FIXME transform function even if abstract version exists (perform copy)
                if ( abstract_function(fn) ) {
                    // if ( lart::tag::has( afn, tag::abstract ) )
                    //     use( call.getInstruction(), afn );
                    llvm_unreachable( "abstraction of functions not supported yet" );
                } else {
                    assert( !fn->isVarArg() && "abstract varargs are not yet supported" );
                    // unify call argument and function argument abstraction
                    forward_use( use.get(), fn->getArg( use.getOperandNo() ) );
                }
            }
        };

        auto calluse = [&] (const llvm::Use &use) {
            auto call = llvm::cast< llvm::CallBase >( use.getUser() );
            if ( use.get() == call->getCalledOperand() )
                forward_use( use.get(), use.getUser() ); // use of function return value
            else
                arguse( use );
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

        auto forward = [ & ](const llvm::Use &use) {
            sc::llvmcase( use.getUser(),
                [&] ( llvm::IntrinsicInst * i ) {
                    if ( keep_intrinsic( i->getIntrinsicID() ) )
                        return;
                    spdlog::info( "ignoring intrinsic: {}", sc::fmt::llvm_to_string( i ));
                },
                [&] ( llvm::CallBase*           ) { calluse( use ); },
                [&] ( llvm::Instruction  * inst ) { forward_use( v, inst ); },
                [&] ( llvm::ConstantExpr * expr ) { forward_use( v, expr ); },
                [&] ( llvm::GlobalVariable * gv ) { forward_use( v, gv ); },
                []  ( llvm::Value *val ) {
                    std::string msg = "unsupported edge"s + sc::fmt::llvm_to_string( val );
                    llvm_unreachable( msg.c_str() );
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

        auto uses = [&] ( auto val ) {
            if ( auto s = llvm::dyn_cast< llvm::StoreInst >(val) ) {
                for ( auto p : aliases.pointsto( s->getPointerOperand() ) ) {
                    push( store_edge( v, p ) );
                }
            }
            /*if ( auto aml = dfg.gv_to_aml( node ) )
                return query::query( aml->succs )
                    .filter( std::not_fn( in_abstractable_function ) )
                    .filter( std::not_fn( in_lava ) )
                    .filter( std::not_fn( in_lamp ) )
                    .freeze();*/
            return val->uses();
        };

        for (const auto &u : uses(v))
            forward( u );

        return edges;
    }

    edges_t dataflow_analysis::induced_edges( llvm::Value * lhs, llvm::Value * rhs )
    {
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
            [&] ( llvm::StoreInst * s )
            {
                if ( lhs == s->getValueOperand() )
                {
                    for ( auto p : aliases.pointsto( s->getPointerOperand() ) )
                        push( store_edge( lhs, p ) );
                    // TODO: stores.insert( s );
                }
            },
            [&] ( llvm::ReturnInst * r )
            {
                assert( lhs == r->getReturnValue() );
                push( unif_edge( r->getReturnValue(), sc::get_function( rhs ) ) );
            },
            [&] ( llvm::Function * )
            {
                // ASSERT( lart::tag::has( fn, tag::abstract ) );
                push( unif_edge( rhs, lhs ) );
            },
            [] ( llvm::BranchInst *)    { /* ignore */ },
            [] ( llvm::ConstantData * ) { /* ignore */ },
            [] ( llvm::Value * v )
            {
                std::string msg = "unsupported edge"s + sc::fmt::llvm_to_string( v );
                llvm_unreachable( msg.c_str() );
            }
        );

        return edges;
    }

    type_map dataflow_analysis::run_from( const roots_map &roots )
    {
        spdlog::debug( "[dfa] setup svf module" );
        auto svfmodule = SVF::LLVMModuleSet::getLLVMModuleSet()->buildSVFModule( module );
        svfmodule->buildSymbolTableInfo();
        assert( svfmodule != nullptr && "SVF Module is null" );

        SVF::SVFIRBuilder builder;
        auto svfir = builder.build( svfmodule );
        assert( svfir != nullptr && "SVFIR is null" );

        aliases.init( svfir );

        for (const auto  &[call, kind] : roots) {
            types.add( call, kind );
            push( call );
        }

        while (!worklist.empty()) {
            process( pop() );
        }

        return types;
    }

    void dataflow_analysis::process( edge &&e )
    {
        auto to = types[ e.to ];
        auto from = types[ e.from ];

        auto push_change = [&] ( auto val, auto type ) {
            if ( types[val] != type ) {
                types[val] = type;
                push( val );
            }
        };

        auto peel = [&] ( auto v ) { return types[ v ].peel(); };
        auto wrap = [&] ( auto v ) { return types[ v ].wrap(); };

        auto joined = [&] () -> type_onion {
            switch ( e.ty ) {
                case edge::type::uniform:
                    if ( llvm::isa< llvm::CmpInst >( e.to ) )
                        return type_onion{ join( to.back(), from.back() ) };
                    return join( to, from );
                case edge::type::load:     return join( to, peel( e.from ) );
                case edge::type::store:    return join( to, wrap( e.from ) );
                default: llvm_unreachable( "unknown edge type" );
            }
        } ();

        // spdlog::debug( "[dfa] {} v {} = {}", from, to, joined );
        push_change( e.to, joined );
    }

} // namespace lart::dfa::detail
