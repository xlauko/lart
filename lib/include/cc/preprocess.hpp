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
#include <llvm/IR/Module.h>
#include <llvm/IR/GetElementPtrTypeIterator.h>

#include <cc/operation.hpp>

#include <sc/init.hpp>

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include <string>

namespace lart
{
    template< typename T > using generator = cppcoro::generator< T >;

    using operation = lart::op::operation;

    struct preprocessor : sc::with_context
    {
        explicit preprocessor( llvm::Module &m ) : sc::with_context( m ), module( m ) {}

        void run( llvm::Function *fn );

        void lower_cmps( llvm::Function *fn );

        inline static const std::string tag = "lart.abstract.preprocessed";

        llvm::Module &module;
    };

    using dependence = std::pair< llvm::Value*, llvm::Instruction* >;

    generator< dependence > lower_pointer_arithmetic( llvm::GetElementPtrInst *gep );

} // namespace lart
