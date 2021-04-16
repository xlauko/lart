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

#include <llvm/Support/raw_ostream.h>

namespace lart::util
{
    template< typename... Ts >
    bool is_one_of( llvm::Value *v ) {
        return ( llvm::isa< Ts >( v ) || ... );
    }

    // Type is not under Value, therefore we need separate function
    template< typename ... Ts >
    bool is_one_of_types( llvm::Type *t )
    {
        return ( llvm::isa< Ts >( t ) || ... );
    }

    inline bool tag_function_with_metadata( llvm::Function &fn, std::string tag ) {
        if ( fn.getMetadata( tag ) )
            return false;
        auto & ctx = fn.getContext();
        auto dummy = llvm::MDString::get( ctx, "dummy" );
        auto meta = llvm::MDTuple::get( ctx , { dummy } );
        fn.setMetadata( tag, meta );
        return true;
    }

    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

} // namespace lart
