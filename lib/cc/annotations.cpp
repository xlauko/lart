/*
 * (c) 2020 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <cc/annotations.hpp>
#include <numeric>
#include <sc/annotation.hpp>
#include <sc/meta.hpp>

namespace lart
{
    template< typename Value > void annotation_to_metadata( tag_t ns, llvm::Module &m )
    {
        for ( const auto &[ val, ann ] : sc::annotation::enumerate< Value >( m ) )
            sc::meta::set( val, ann.str() );
    }

    template< typename Value >
    void annotation_to_domain_metadata( tag_t ns, llvm::Module &m )
    {
        for ( const auto &[ val, ann ] : sc::annotation::enumerate< Value >( m ) )
            sc::meta::set( val, ns, ann.name() );
    }

    void annotations::lower( llvm::Module &m )
    {
        annotation_to_domain_metadata< llvm::Function >( tag::abstract, m );
        annotation_to_metadata< llvm::Function >( tag::transform::prefix, m );
        annotation_to_metadata< llvm::Function >( tag::noalias::prefix, m );
    }

} // namespace lart
