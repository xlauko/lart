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

    using operation = lart::op::operation;

    namespace detail
    {
        generator< unsigned > liftable_indices( const TestTaint &test )
        {
            auto op = test.op;
            // skip lifter and default value arguments
            unsigned pos = op::returns_value(op) ? 2 : 1;

            for ( auto arg : op::arguments(op) ) {
                if ( arg.liftable ) {
                    co_yield pos;
                    pos += 2;
                } else {
                    pos++;
                }
            }
        }
    }

    generator< llvm::Value* > arguments(const lifter &lif, const operation &op )
    {
        co_yield lif.function();

        if ( auto def = op::default_value(op); def.has_value() )
            co_yield def.value();

        for ( auto arg : op::arguments(op) ) {
            if ( arg.liftable ) {
                co_yield arg.value;
                co_yield op::abstract_pointer();
            } else {
                co_yield arg.value;
            }
        }
    }

    std::string TestTaint::name() const
    {
        return "lart.test.taint." + op::name(op) + "." + op::unique_name_suffix(op);
    }

    llvm::CallInst* TestTaint::intrinsic()
    {
        llvm::IRBuilder<> irb( op::location(op) );

        lifter lif(module, op);
        auto args = sv::freeze( arguments( lif, op ) );

        auto get_or_insert_function = [&] (auto fty, auto name) {
            auto fn = llvm::cast< llvm::Function >(
                module.getOrInsertFunction( name, fty ).getCallee()
            );
            fn->addFnAttr( llvm::Attribute::NoUnwind );
            return fn;
        };

        auto out = op::default_value(op);
        auto rty = out.has_value() ? out.value()->getType() : sc::void_t();
        auto fty = llvm::FunctionType::get( rty, sv::freeze( args | sv::types ), false );
        auto tester = get_or_insert_function( fty, name() );

        return irb.CreateCall( tester, args );
    }

    generator< arg::liftable > liftable_view( const TestTaint &test )
    {
        auto call = test.call;
        for ( auto i : detail::liftable_indices(test) )
            co_yield { call->getOperandUse(i), call->getOperandUse(i + 1) };
    }

} // namespace lart
