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

#include <cc/alias.hpp>
#include <cc/preprocess.hpp>
#include <cc/tristate.hpp>
#include <cc/logger.hpp>
#include <cc/roots.hpp>
#include <cc/util.hpp>

#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/ErrorHandling.h>

#include <sc/init.hpp>
#include <sc/format.hpp>

#include <vector>
#include <cassert>
#include <iostream>
#include <experimental/iterator>
#include <unordered_map>

namespace lart::dfa
{

namespace detail
{
    struct type_layer
    {
        tristate pointer;
        tristate abstract;

        type_layer( bool p, bool a ) : pointer( p ), abstract( a ) {}
        type_layer( tristate p, tristate a ) : pointer( p ), abstract( a ) {}

        auto operator<=>( const type_layer & ) const = default;

        template< typename stream >
        friend auto operator<<( stream &s, type_layer t ) -> decltype( s << "" )
        {
            return s << "ptr:" << t.pointer
                     << " abs:" << t.abstract;
        }
    };

    inline type_layer join( type_layer a, type_layer b )
    {
        return { join( a.pointer, b.pointer ), join( a.abstract, b.abstract ) };
    }

    using type_vector = std::vector< type_layer >;

    struct type_onion : type_vector
    {
        type_onion( size_t ptr_nest )
            : type_vector( ptr_nest + 1, type_layer( true, false ) )
        {
            front() = type_layer( false, false );
        }

        type_onion( const type_vector &l ) : type_vector( l ) {}
        type_onion( std::initializer_list< type_layer > il ) : type_vector( il ) {}

        type_onion make_abstract() const
        {
            auto rv = *this;
            rv.back().abstract = tristate(true);
            return rv;
        }

        type_onion make_pointer() const
        {
            auto rv = *this;
            rv.back().pointer = tristate(true);
            return rv;
        }

        type_onion make_abstract_pointer() const
        {
            auto on = this->make_abstract().make_pointer();
            on.front().abstract = tristate::maybe;
            on.front().pointer = tristate::maybe;
            return on;
        }

        bool maybe_abstract() const
        {
            tristate r( false );
            for ( auto a : *this )
                r = join( r, a.abstract );
            return r.value != tristate::no;
        }

        bool maybe_pointer() const
        {
            tristate r( false );
            for ( auto a : *this )
                r = join( r, a.pointer );
            return r.value != tristate::no;
        }

        type_onion wrap() const
        {
            auto rv = *this;
            rv.emplace_back( true, false );
            return rv;
        }

        type_onion peel() const
        {
            auto rv = *this;
            if ( size() == 1 )
                rv.front().pointer = tristate::maybe;
            else
            {
                assert( back().pointer != tristate( false ) );
                rv.pop_back();
            }
            return rv;
        }

        template< typename stream >
        friend auto operator<<( stream &s, type_onion t ) -> decltype( s << "" )
        {
            s << '[';
            std::copy(t.begin(), t.end(), std::experimental::make_ostream_joiner(s, ", "));
            s << "]";
            return s;
        }
    };

    inline type_onion join( type_onion a, type_onion b )
    {
        if ( a.size() > b.size() )
            std::swap( a, b );

        if ( a.size() < b.size() )
        {
            for ( size_t i = 0; i < b.size() - a.size(); ++i )
                a.front() = join( a.front(), b[ i ] );
            a.front().pointer = tristate::maybe;
        }

        for ( size_t i = 0; i < a.size(); ++i )
            a[ i ] = join( a[ i ], b[ i + b.size() - a.size() ] );

        return a;
    }

    using onion_map = std::unordered_map< llvm::Value *, type_onion >;

    struct type_map : onion_map
    {
        using type = type_onion;

        using onion_map::count;
        using onion_map::emplace;
        using onion_map::insert_or_assign;

        size_t pointer_nesting( llvm::Type *t ) const noexcept
        {
            size_t rv = 0;
            while ( t->isPointerTy() )
                ++rv, t = t->getPointerElementType();
            return rv;
        }

        type_onion add( llvm::Value * v, abstract_kind kind )
        {
            auto &onion = get( v );
            switch ( kind ) {
            case abstract_kind::scalar:
                onion = onion.make_abstract(); break;
            case abstract_kind::pointer:
                onion = onion.make_abstract_pointer(); break;
            default: llvm_unreachable( "unsupported onion kind" );
            }

            return onion;
        }

        [[nodiscard]] type_onion& get( llvm::Value * v )
        {
            if ( !count( v ) ) {
                auto ty = v->getType();
                if ( auto fn = llvm::dyn_cast< llvm::Function >( v ) )
                    ty = fn->getReturnType();
                emplace( v, type_onion( pointer_nesting( ty ) ) );
            }
            return this->at( v );
        }

        type_onion& operator[]( llvm::Value * v ) { return get( v ); }
        const type_onion& operator[]( llvm::Value * v ) const { return this->at( v ); }
    };

    struct edge
    {
        enum class type { uniform, load, store };

        llvm::Value * from;
        llvm::Value * to;
        type ty;

        template< typename stream >
        friend auto operator<<( stream &s, edge e ) -> decltype( s << "" )
        {
            auto llvm_place = [] ( llvm::Value * v ) -> std::string {
                if ( auto glob = llvm::isa< llvm::GlobalVariable >( v ) )
                    return "global";
                if ( llvm::isa< llvm::Function >( v ) )
                    return "function";
                if ( llvm::isa< llvm::ConstantExpr >( v ) )
                    return "cexpr";
                if ( util::is_one_of< llvm::Argument, llvm::Instruction >( v ) )
                    return sc::get_function( v )->getName().str();
                llvm_unreachable( "unknown llvm place" );
            };

            auto pf = llvm_place( e.from );
            auto pt = llvm_place( e.to );

            if ( pf == pt ) {
                return s << pf << ":" << sc::fmt::llvm_name( e.from ) << " → "
                         << sc::fmt::llvm_name( e.to );
            }
            return s << pf << ":" << sc::fmt::llvm_name( e.from ) << " → "
                     << pt << ":" << sc::fmt::llvm_name( e.to );
        }
    };

    using edges_t = std::vector< edge >;

    struct dataflow_analysis : sc::with_context
    {
        explicit dataflow_analysis( llvm::Module &m )
            : sc::with_context( m ), module( m ), prep( m )
        {}

        void push( edge &&e ) noexcept;
        void push( llvm::Value *v ) noexcept;
        edge pop() noexcept;

        void process( edge &&e );

        edges_t edges( llvm::Value * v );
        edges_t induced_edges( llvm::Value * lhs, llvm::Value * rhs );

        sc::generator< llvm::Function * > destinations( llvm::CallBase *call );

        void preprocess( llvm::Function * ) const;

        type_map run_from( const roots_map &roots );

        std::queue< edge > worklist;
        type_map types;

        aa::andersen aliases;

        llvm::Module &module;
        preprocessor prep;
    };

} // namespace lart::detail

    using types = detail::type_map;

    struct analysis
    {
        explicit analysis( sc::module_ref m ) : impl( m ) {}

        static types run_on( sc::module_ref m )
        {
            spdlog::debug( "[dfa] start dataflow analysis" );
            analysis dfa( m );
            auto roots = gather_roots( m );
            return dfa.impl.run_from( roots );
        }

        detail::dataflow_analysis impl;
    };
} // namespace lart::dfa
