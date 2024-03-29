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

#include <cc/ir.hpp>

#include <variant>

namespace lart::backend
{
    struct base
    {
        struct testtaint { };

        virtual ~base() = default;

        void lower( ir::intrinsic i );
        void lower( llvm::CallInst *call, op::operation );

        using callinst = llvm::CallInst*;
        virtual void lower_test_taint( ir::intrinsic ) = 0;

        virtual void lower( callinst c, op::freeze f ) = 0;
        virtual void lower( callinst c, op::unstash u ) = 0;
        virtual void lower( callinst c, op::unstash_taint u ) = 0;
        virtual void lower( callinst c, op::stash s ) = 0;

        virtual void lower( callinst c, op::dump s ) = 0;
    };

} // namespace lart::backend
