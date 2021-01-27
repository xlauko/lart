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

#include <cc/operation.hpp>

#include <bitset>
#include <external/coro/generator.hpp>

namespace lart
{
    template< typename T > using generator = cppcoro::generator< T >;

    struct TestTaint
    {
        using operation = lart::op::operation;

        explicit TestTaint( llvm::Module &m, operation o )
            : module( m ), op( o ), call( intrinsic() )
        {}

    private:
        llvm::CallInst * intrinsic();
        std::string name() const;

        llvm::Module &module;

    public:
        operation op;
        llvm::CallInst *call;
    };

    namespace arg
    {
        struct liftable
        {
            llvm::Use &concrete;
            llvm::Use &abstract;
        };
    } // namespace arg

    generator< arg::liftable > liftable_view( const TestTaint &test );

} // namespace lart
