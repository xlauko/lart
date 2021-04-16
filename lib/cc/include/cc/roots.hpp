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

#include <sc/annotation.hpp>
#include <sc/case.hpp>

#include <llvm/IR/CallSite.h>
#include <llvm/Support/ErrorHandling.h>

#include <map>
#include <queue>

namespace lart
{
    enum class abstract_kind { scalar, pointer };

    using roots_map = std::map< llvm::CallSite, abstract_kind >;

    inline abstract_kind annotated_kind( const sc::annotation &ann )
    {
        if ( ann.name() == "scalar" )
            return abstract_kind::scalar;
        if ( ann.name() == "pointer" )
            return abstract_kind::pointer;
        llvm_unreachable( "unknown annotation kind" );
    }

    inline roots_map gather_roots( llvm::Module &m )
    {
        roots_map result;
        /*auto ns = sc::annotation("lart", "abstract", "return");
        auto annotated = sc::annotation::enumerate_in_namespace< llvm::Function >(ns, m);


        for ( const auto &[fn, ann] : annotated )
        {
            std::queue< llvm::Value* > worklist;
            auto transitive_users = [&] (auto val) {
                for ( auto user : val->users() )
                    worklist.emplace( user );
            };

            auto kind = annotated_kind( ann );

            transitive_users(fn);
            while ( !worklist.empty() ) {
                sc::llvmcase( worklist.front(),
                    [&] ( llvm::CastInst *c )     { transitive_users(c); },
                    [&] ( llvm::ConstantExpr *c ) { transitive_users(c); },
                    [&] ( llvm::CallInst *c )     { result[c] = kind; },
                    [&] ( llvm::InvokeInst * c)   { result[c] = kind; }
                );
                worklist.pop();
            }
        }*/

        for ( auto &fn : m )
        {
            if ( fn.hasName() && !(
                fn.getName().startswith( "__lamp_lift" ) ||
                fn.getName().startswith( "__lamp_any" )
               ) )
               continue;

            std::queue< llvm::Value* > worklist;
            auto transitive_users = [&] (auto val) {
                for ( auto user : val->users() )
                    worklist.emplace( user );
            };

            auto kind = abstract_kind::scalar;

            transitive_users(&fn);
            while ( !worklist.empty() ) {
                sc::llvmcase( worklist.front(),
                    [&] ( llvm::CastInst *c )     { transitive_users(c); },
                    [&] ( llvm::ConstantExpr *c ) { transitive_users(c); },
                    [&] ( llvm::CallInst *c )     { result[c] = kind; },
                    [&] ( llvm::InvokeInst * c)   { result[c] = kind; }
                );
                worklist.pop();
            }
        }
        return result;
    }

} // namespace lart

