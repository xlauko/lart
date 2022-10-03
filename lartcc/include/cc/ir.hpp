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

#include <sc/query.hpp>

#include <variant>

namespace lart::ir
{
    using operation = lart::op::operation;

    struct intrinsic
    {
        llvm::CallInst *call;
        operation op;
    };

    namespace arg
    {
        struct with_taint
        {
            llvm::Use &taint;
            llvm::Use &concrete;
            llvm::Use &abstract;
        };

        struct without_taint_concrete
        {
            llvm::Use &value;
        };

        struct without_taint_abstract
        {
            llvm::Use &concrete;
            llvm::Use &abstract;
        };

    } // namespace arg

    using argument = std::variant< arg::with_taint, arg::without_taint_concrete, arg::without_taint_abstract >;

    namespace detail
    {
        static auto invoke = [] (auto f) { return [=] ( auto a ) { return std::visit(f, a); }; };
    } // namespace detail

} // namespace lart::ir
