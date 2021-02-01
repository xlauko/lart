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

#include <llvm/IR/Instructions.h>

#include <cc/operation.hpp>

#include <variant>

namespace lart::backend
{
    struct intrinsic_base
    {
        llvm::CallInst *call;
    };

    struct stash : intrinsic_base, op::unstash_base {};
    struct unstash : intrinsic_base, op::unstash_base {};


    using intrinsic = std::variant< stash, unstash >;

    struct base
    {
        virtual ~base() = default;

        void lower( intrinsic i );

        std::optional< intrinsic > get_intrinsic( llvm::CallInst *call );

        virtual void lower( stash u )   = 0;
        virtual void lower( unstash u ) = 0;
    };

} // namespace lart::backend
