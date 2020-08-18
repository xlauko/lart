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

#pragma once

#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Module.h>

namespace lart
{
    using ctag_t = llvm::StringLiteral;

    using tag_t      = llvm::StringRef;
    using meta_t     = llvm::StringRef;
    using maybe_meta = std::optional< std::string >;

    namespace tag
    {
        namespace noalias
        {
            constexpr ctag_t prefix = "lart.noalias";
            constexpr ctag_t arg    = "lart.noalias.arg";
            constexpr ctag_t ret    = "lart.noalias.ret";
        } // namespace noalias

        namespace function
        {
            constexpr ctag_t arguments = "lart.function.argument.tags";
        } // namespace function

        namespace transform
        {
            constexpr ctag_t prefix = "lart.transform";

            namespace ignore
            {
                constexpr ctag_t ret = "lart.transform.ignore.ret";
                constexpr ctag_t arg = "lart.transform.ignore.arg";
            } // namespace ignore

            constexpr ctag_t forbidden = "lart.transform.forbidden";
        } // namespace transform

    } // namespace tag

    struct annotation
    {
        template< typename... parts_t > constexpr explicit annotation( parts_t... parts )
        {
            ( _parts.push_back( parts ), ... );
        }

        const std::string &back() const;

        std::string str() const;

        std::vector< std::string > _parts;
    };

    struct annotations
    {
        static void lower( llvm::Module &m );
    };

} // namespace lart
