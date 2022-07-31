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

#include <cc/runtime.hpp>


namespace lart::runtime
{
    void initialize(sc::module_ref &mod) {
        lamp_runtime_generator lamp(mod);

        using primitive = std::pair< std::string, sc::type >;
        std::array primitives = {
            primitive{"i1", sc::i1()},
            primitive{"i8", sc::i8()},
            primitive{"i16", sc::i16()},
            primitive{"i32", sc::i32()},
            primitive{"i64", sc::i64()},
        };

        auto register_for_primitives = [&] (auto reg) {
            for (const auto &[name, type] : primitives) {
                reg(name, type);
            }
        };

        register_for_primitives([&] (const auto &name, auto to) {
            lamp.register_any(name, to);
        });

        register_for_primitives([&] (const auto &name, auto from) {
            lamp.register_lift(name, from);
        });

        register_for_primitives([&] (const auto &name, auto from) {
            lamp.register_wrap(name, from);
        });

        std::array binary = {
            "add", "sub", "mul", "sdiv", "udiv", "srem", "urem"
        };

        for (const auto &bin : binary) {
            lamp.register_binary(bin);
        }

        lamp.register_operation("freeze", sc::void_t(), {
            lamp.abstract_type(), lamp.abstract_type(), lamp.bitwidth()
        });

        lamp.register_operation("melt", lamp.abstract_type(), {
            lamp.abstract_type(), lamp.bitwidth()
        });

        lamp.register_operation("store", sc::void_t(), {
            lamp.abstract_type(), lamp.abstract_type(), lamp.bitwidth()
        });

        lamp.register_operation("load", lamp.abstract_type(), {
            lamp.abstract_type(), lamp.bitwidth()
        });
    }

} // namespace lart::runtime
