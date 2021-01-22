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

#include <sc/ranges.hpp>

namespace lart
{
    namespace sv = sc::views;

    bool is_abstract_pointer( const dfa::types::type &type )
    {
        return static_cast< bool >( type.back().pointer );
    }
    operation syntactic::make_operation( llvm::Value *val )
    {
        operation result;
        val->dump();
        sc::llvmcase( val,
            [&] ( llvm::AllocaInst * ) {
                if ( is_abstract_pointer(types[val]) )
                    result = op::alloc(val);
            },
            [&] ( llvm::LoadInst * ) {
                if ( is_abstract_pointer(types[val]) )
                    result = op::load(val);
                else
                    result = op::melt(val);
            },
            [&] ( llvm::BinaryOperator * ) {
                result = op::binary( val );
            },
            [&] ( llvm::Value * ) {
                __builtin_unreachable();
            }
        );
        return result;
    }

    std::vector< operation > syntactic::toprocess()
    {
        std::vector< operation > ops;
        for ( const auto &[val, type] : types )
            ops.emplace_back( make_operation(val) );

        for ( auto store : sv::filter< llvm::StoreInst >( module ) ) {
            store->dump();
        }
        return ops;
    }

    void syntactic::process( const operation & )
    {

    }

} // namespace lart
