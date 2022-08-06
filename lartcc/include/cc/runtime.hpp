/*
 * (c) 2022 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <llvm/IR/Module.h>

#include <sc/context.hpp>
#include <sc/init.hpp>
#include <sc/ir.hpp>
#include <sc/builder.hpp>
#include <sc/types.hpp>

#include <llvm/IR/Attributes.h>

#include <vector>

namespace lart::runtime
{
    using function_type  = llvm::FunctionType *;
    using attribute      = llvm::Attribute;

    static inline attribute abstract_scalar_attr() {
        return attribute::get(sc::context(), "lamp-scalar-value");
    }

    static inline attribute abstract_pointer_attr() {
        return attribute::get(sc::context(), "lamp-pointer-value");
    }

    struct lamp_runtime_generator : sc::with_context {

        lamp_runtime_generator(sc::module_ref m)
            : sc::with_context(m), mod(m)
        {}

        sc::type abstract_type() { return sc::i8p(); }
        sc::type bitwidth() { return sc::i8(); }

        std::vector< sc::type > args(std::size_t n) {
            return std::vector(n, abstract_type());
        }

        sc::function set_abstract_return_attr(sc::function fn) {
            fn->addFnAttr(abstract_scalar_attr());
            return fn;
        }

        sc::function insert_operation(const std::string &name, function_type fty) {
            return llvm::cast< llvm::Function >(
                mod.getOrInsertFunction("__lamp_" + name, fty).getCallee()
            );
        }

        sc::function insert_and_annotate_operation(const std::string &name, function_type fty) {
            return set_abstract_return_attr(insert_operation(name, fty));
        }

        sc::function register_binary(const std::string &name) {
            auto fty = llvm::FunctionType::get( abstract_type(), args(2), false );
            return insert_operation(name, fty);
        }

        sc::function register_any(const std::string &name, sc::type to) {
            auto fty = llvm::FunctionType::get( to, {}, false );
            return insert_and_annotate_operation("any_" + name, fty);
        }

        sc::function register_lift(const std::string &name, sc::type from) {
            auto fty = llvm::FunctionType::get( abstract_type(), { from }, false );
            return insert_operation("lift_" + name, fty);
        }

        sc::function register_wrap(const std::string &name, sc::type from) {
            auto fty = llvm::FunctionType::get( abstract_type(), { from }, false );
            return insert_operation("wrap_" + name, fty);
        }

        sc::function register_operation(const std::string &name, sc::type rty, const std::vector< sc::type > &args) {
            auto fty = llvm::FunctionType::get( rty, args, false );
            return insert_operation(name, fty);
        }

        sc::function register_cast(const std::string &name) {
            auto fty = llvm::FunctionType::get( abstract_type(), { abstract_type(), bitwidth() }, false );
            return insert_operation(name, fty);
        }

        sc::module_ref mod;
    };

    void initialize( llvm::Module &module );
} // namespace lart::runtime
