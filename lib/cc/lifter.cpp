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

#include <cc/lifter.hpp>

#include <sc/ranges.hpp>

namespace lart
{
    namespace sv = sc::views;

    std::string lifter::name() const
    {
        return "lart.lifter." + op::name(op) + "." + op::unique_name_suffix(op);
    }

    llvm::Function* lifter::function() const
    {
        auto aptr = op::abstract_pointer()->getType();

        std::vector< llvm::Type * > args;
        for ( auto arg : op::arguments(op) ) {
            if ( arg.liftable ) {
                args.push_back(sc::i1()); // dummy false
                args.push_back(arg.value->getType());
                args.push_back(aptr);
            } else {
                args.push_back(arg.value->getType());
            }
        }

        return op::intrinsic( op, &module, args, name() );
    }

} // namespace lart
