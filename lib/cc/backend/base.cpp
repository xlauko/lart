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

#include <cc/backend/base.hpp>

namespace lart::backend
{
    bool intrinsic_base::is_test_taint() const
    {
        return call->getCalledFunction()->getName().startswith( "lart.test.taint" );
    }

    void base::lower( intrinsic i )
    {
        std::visit( [&](auto &&a) { this->lower(a); }, i );
    }

    template< typename intr >
    auto isintrinsic = [] (auto name) -> bool {
        return name.startswith( "lart." + intr::name ) ||
               name.startswith( "lart.test.taint." + intr::name );
    };

    std::optional< intrinsic > base::get_intrinsic( llvm::CallInst *call )
    {
        if ( auto fn = call->getCalledFunction() ) {
            if ( !fn->hasName() )
                return std::nullopt;
            auto name = fn->getName();
            if ( isintrinsic< stash >( name ) )
                return unstash{ call, {} };
            if ( isintrinsic< unstash >( name ) )
                return unstash{ call, {} };
        }
        return std::nullopt;
    }

} // namespace lart::backend
