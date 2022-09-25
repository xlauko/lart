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

#include <cc/arguments.hpp>
#include <cc/taint.hpp>
#include <cc/lifter.hpp>

#include <sc/query.hpp>

namespace lart::taint
{
    namespace detail
    {
        std::string name( const operation &op )
        {
            return "lart.test.taint." + op::name(op) + "." + op::unique_name_suffix(op);
        }

        sc::generator< sc::value > arguments( const lart::lifter &lifter )
        {
            co_yield lifter.function();

            for ( auto arg : op::duplicated_arguments(lifter.op, lifter.shadows) ) {
                co_yield arg;
            }
        }

        sc::generator< unsigned > paired_indices( const ir::intrinsic &intr )
        {
            auto op = intr.op;
            // skip lifter argument
            unsigned pos = op::with_taints(op) ? 1 : 0;

            for ( auto arg : op::arguments(op) ) {
                switch ( arg.type ) {
                case op::argtype::abstract:
                case op::argtype::lift:
                    co_yield pos;
                    pos += 3;
                    break;
                case op::argtype::test:
                case op::argtype::concrete:
                    pos += 1;
                    break;
                }
            }
        }

    } // namespace detail

    llvm::CallInst * make_call( const lifter &lift )
    {
        std::vector< sc::value > args;
        for (auto arg : detail::arguments( lift )) {
            args.push_back(arg);
        }
        return op::make_call( lift.op, args, detail::name( lift.op ) );
    }

    sc::generator< ir::arg::tuple > paired_view( const ir::intrinsic &test )
    {
        auto call = test.call;
        for ( auto i : detail::paired_indices( test ) ) {
            co_yield {
                call->getOperandUse(i),
                call->getOperandUse(i + 1),
                call->getOperandUse(i + 2)
            };
        }
    }

} // namespace lart::taint
