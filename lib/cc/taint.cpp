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

#include <cc/taint.hpp>
#include <cc/lifter.hpp>

#include <sc/ranges.hpp>

namespace lart
{
    namespace sv = sc::views;

    auto test_taint_call::arguments() const -> args_t
    {
        auto lift = lifter(module, op).function();
        args_t args = { lift, op::default_value(op) };

        auto s = op::split_arguments(op);
        args.insert(args.end(), std::make_move_iterator(s.begin())
                              , std::make_move_iterator(s.end()));

        return args;
    }

    std::string test_taint_call::name() const
    {
        return "lart.test.taint." + op::name(op) + "." + op::unique_name_suffix(op);
    }

    llvm::Function* test_taint_call::tester() const
    {
        auto get_or_insert_function = [&] (auto fty, auto name) {
            auto fn = llvm::cast< llvm::Function >(
                module.getOrInsertFunction( name, fty ).getCallee()
            );
            fn->addFnAttr( llvm::Attribute::NoUnwind );
            return fn;
        };

        auto out = op::default_value(op);
        auto args = arguments();

        auto fty = llvm::FunctionType::get( out->getType(), sv::freeze( args | sv::types ), false );
        return get_or_insert_function( fty, name() );
    }

    llvm::CallInst* test_taint_call::build() const
    {
        llvm::IRBuilder<> irb( op::location(op) );
        tester()->dump();
        return nullptr;
    }

} // namespace lart
