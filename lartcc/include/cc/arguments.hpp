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

#include <cc/operation.hpp>
#include <cc/shadow.hpp>

#include <sc/generator.hpp>

namespace lart::op {

    static inline sc::generator< sc::value > duplicated_arguments(const operation &op, const shadow_map &shadows)
    {
        for ( auto arg : op::arguments(op) ) {
            switch (arg.type) {
                case argtype::abstract:
                    co_yield arg.value;
                    co_yield op::abstract_pointer();
                    break;
                case argtype::lift:
                    co_yield shadows.get(arg.value);
                    co_yield arg.value;
                    co_yield op::abstract_pointer();
                    break;
                case argtype::test:
                    co_yield shadows.get(arg.value);
                    break;
                case argtype::concrete:
                    co_yield arg.value;
            }
        }
    }

} // namesapce lart::op
