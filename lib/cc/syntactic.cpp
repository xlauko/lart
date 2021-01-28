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

#include <cc/syntactic.hpp>

#include <cc/logger.hpp>
#include <cc/taint.hpp>

#include <sc/ranges.hpp>



namespace lart
{
    namespace sv = sc::views;

    bool is_abstract( const dfa::types::type &type )
    {
        return static_cast< bool >( type.back().abstract );
    }

    bool is_abstract_pointer( const dfa::types::type &type )
    {
        return static_cast< bool >( type.back().pointer ) && is_abstract( type );
    }

    bool is_lamp_call( llvm::CallSite call )
    {
        if ( call.isIndirectCall() )
            return false;
        return call.getCalledFunction()->getName().startswith( "__lamp" );
    }

    std::optional< operation > syntactic::make_operation( llvm::Value *val )
    {
        std::optional< operation > result;
        sc::llvmcase( val,
            [&] ( llvm::AllocaInst * ) {
                if ( is_abstract_pointer( types[val] ) )
                    result = op::alloc(val);
            },
            [&] ( llvm::LoadInst * ) {
                if ( is_abstract_pointer( types[val] ) )
                    result = op::load(val);
                else
                    result = op::melt(val);
            },
            [&] ( llvm::StoreInst *store ) {
                if ( is_abstract_pointer( types[ store->getPointerOperand() ] ) )
                    result = op::store(val);
                else if ( is_abstract( types[ store->getValueOperand() ] ) )
                    result = op::freeze(val);
            },
            [&] ( llvm::BinaryOperator * ) {
                result = op::binary( val );
            },
            [&] ( llvm::CallInst *call ) {
                if ( is_lamp_call(call) ) {
                    result = op::unstash(call);
                } else {
                    // TODO
                }
            },
            [&] ( llvm::Value * ) { /* fallthrough */ }
        );
        return result;
    }

    generator< operation > syntactic::toprocess()
    {
        for ( const auto &[val, type] : types )
            if ( auto op = make_operation(val); op.has_value() )
                co_yield op.value();

        for ( auto store : sv::filter< llvm::StoreInst >( module ) ) {
            auto ptr = store->getPointerOperand();
            if ( types.count(ptr) && types[ptr].maybe_abstract() )
                if ( auto op = make_operation(store); op.has_value() )
                    co_yield op.value();
        }
    }

    void syntactic::process( operation o )
    {
        spdlog::info( "process {}", op::name(o) );

        auto test = TestTaint( module, o );

        if ( op::returns_value(o) ) {
            auto concrete = op::value(o);
            abstract[concrete] = test.call;

            // Update placeholders where result of this operation
            // should be used instead. That are arguments of already
            // abstracted users of concrete variant of this operation.
            if ( auto args = places.find( concrete ); args != places.end() ) {
                for ( auto &place : args->second )
                    place.abstract.set( test.call );
                places.erase(args);
            }
        }

        // Update placeholders of this instruction. If the abstract
        // instruction does not exist yet, store the placeholder
        // to quickly find it when abstract value is generated later.
        for ( auto arg : liftable_view(test) )
        {
            auto &con = arg.concrete;
            if ( auto abs = abstract.find( con.get() ); abs != abstract.end() )
                arg.abstract.set( abs->second );
            else
                places[con].push_back( arg );
        }
    }

} // namespace lart
