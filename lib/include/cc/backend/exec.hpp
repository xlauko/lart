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

#include <cc/backend/base.hpp>
#include <llvm/IR/Module.h>

#include <sc/types.hpp>

namespace lart::backend
{
    struct exec : base
    {
        exec( llvm::Module &m ) : module( m )
        {
            auto getfunction = [&] ( auto name, auto fty ) {
                return llvm::cast< llvm::Function >(
                    module.getOrInsertFunction( name, fty ).getCallee()
                );
            };

            unstash_fn = getfunction( "__lart_unstash",
                llvm::FunctionType::get( sc::i8p(), {}, false )
            );
        }

        using base::lower;

        void lower( unstash ) override;

    private:

        llvm::Function *unstash_fn;

        llvm::Module &module;
    };
}
