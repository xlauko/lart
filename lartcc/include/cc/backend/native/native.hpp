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
#include <cc/operation.hpp>

#include <cc/shadow.hpp>

#include <llvm/IR/Module.h>

#include <sc/types.hpp>

namespace lart::backend
{
    struct native : base
    {
        native( llvm::Module &m ) : module( m )
        {
            auto getfunction = [&] ( auto name, auto fty ) {
                return llvm::cast< llvm::Function >(
                    module.getOrInsertFunction( name, fty ).getCallee()
                );
            };

            // TODO unify with operation impl
            stash_fn = getfunction( "__lart_stash",
                llvm::FunctionType::get( sc::void_t(), { sc::i1(), sc::i8p() }, false )
            );

            unstash_fn = getfunction( "__lart_unstash",
                llvm::FunctionType::get( sc::i8p(), {}, false )
            );

            unstash_taint_fn = getfunction( "__lart_unstash_taint",
                llvm::FunctionType::get( sc::i1(), {}, false )
            );

            freeze_fn = getfunction( "__lamp_freeze",
                llvm::FunctionType::get( sc::void_t(), { sc::i8p(), sc::i8p(), sc::i64() }, false )
            );

            dump_fn = getfunction( "__lamp_dump",
                llvm::FunctionType::get( sc::void_t(), { sc::i8p() }, false )
            );
        }

        using base::lower;
        using base::testtaint;
        using base::callinst;

        void lower_test_taint( ir::intrinsic ) override;

        void lower( callinst c, op::freeze f ) override;
        void lower( callinst c, op::unstash u ) override;
        void lower( callinst c, op::unstash_taint u ) override;
        void lower( callinst c, op::stash s ) override;

        void lower( callinst c, op::dump s ) override;

    private:
        llvm::Function *stash_fn;
        llvm::Function *unstash_fn;
        llvm::Function *unstash_taint_fn;
        llvm::Function *freeze_fn;
        llvm::Function *dump_fn;

        llvm::Module &module;
    };
}
