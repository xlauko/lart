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

namespace lart::taint
{
    namespace sv = sc::views;
    namespace detail
    {
        std::string name( const operation &op )
        {
            return "lart.test.taint." + op::name(op) + "." + op::unique_name_suffix(op);
        }

        generator< llvm::Value* > arguments( const lart::lifter &lifter, const operation &op )
        {
            co_yield lifter.function();

            if ( auto def = op::default_value(op); def.has_value() )
                co_yield def.value();

            for ( auto arg : op::duplicated_arguments(op) )
                co_yield arg;
        }

        generator< unsigned > liftable_indices( const ir::intrinsic &test )
        {
            auto op = test.op;
            // skip lifter and default value arguments
            unsigned pos = op::returns_value(op) ? 2 : 1;

            for ( auto arg : op::arguments(op) ) {
                switch ( arg.type ) {
                case op::argtype::lift:
                    co_yield pos;
                    pos += 2;
                    break;
                case op::argtype::test:
                case op::argtype::concrete:
                    pos++;
                    break;
                }
            }
        }

    } // namespace detail

    llvm::CallInst * make_call( llvm::Module &module, const operation &op )
    {
        lart::lifter lifter( module, op );
        auto args = sv::freeze( detail::arguments( lifter, op ) );
        return op::make_call( op, args, detail::name( op ) );
    }

    generator< ir::arg::liftable > liftable_view( const ir::intrinsic &test )
    {
        // TODO assert is test taint
        auto call = test.call;
        for ( auto i : detail::liftable_indices( test ) )
            co_yield { call->getOperandUse(i), call->getOperandUse(i + 1) };
    }

} // namespace lart::taint
