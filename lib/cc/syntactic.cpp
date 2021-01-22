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

namespace lart
{

    bool is_abstract_pointer( const dfa::types::type &type )
    {
        return static_cast< bool >( type.back().pointer );
    }

    std::vector< operation > syntactic::toprocess( dfa::types map )
    {
        std::vector< operation > ops;
        for ( const auto &[val, type] : map ) {
            sc::llvmcase( val,
                [&] ( llvm::AllocaInst * ) {
                    if ( is_abstract_pointer(type) ) {
                        ops.emplace_back( op::alloc(val) );
                    }
                },
                [&] ( llvm::LoadInst * ) {
                    if ( is_abstract_pointer(type) )
                        ops.emplace_back( op::load(val) );
                    else
                        ops.emplace_back( op::melt(val) );
                },
                [&] ( llvm::BinaryOperator * ) {
                    ops.emplace_back( op::binary( val ) );
                }
            );
        }
        return ops;
    }

    void syntactic::process( const operation & )
    {

    }

} // namespace lart
