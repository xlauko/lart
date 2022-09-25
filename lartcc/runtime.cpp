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
        runtime_generator runtime(mod);

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
            runtime.register_any(name, to);
        });

        register_for_primitives([&] (const auto &name, auto from) {
            runtime.register_lift(name, from);
        });

        register_for_primitives([&] (const auto &name, auto from) {
            runtime.register_wrap(name, from);
        });

        std::array binary = {
            "add", "sub", "mul", "sdiv", "udiv", "srem", "urem",

            "fadd", "fsub", "fmul", "fdiv", "frem",

            "shl", "ashr", "lshr", "and", "or", "xor",

            "eq", "ne", "ugt", "uge", "ult", "ule", "sgt", "sge", "slt", "sle",

            "foeq", "fogt", "foge", "folt", "fole", "fone", "ford", "funo",
            "fueq", "fugt", "fuge", "fult", "fule", "fune", "ffalse", "ftrue"

            "concat"
        };

        for (const auto &bin : binary) {
            runtime.register_binary(bin);
        }

        std::array casts = {
            "trunc", "fptrunc", "sitofp", "uitofp", "zext", "sext", "fpext", "fptosi", "fptoui"
        };

        for (const auto &cast : casts) {
            runtime.register_cast(cast);
        }

        runtime.register_operation("freeze", sc::void_t(), {
            runtime.abstract_type(), runtime.abstract_type(), runtime.bitwidth()
        });

        runtime.register_operation("melt", runtime.abstract_type(), {
            runtime.abstract_type(), runtime.bitwidth()
        });

        runtime.register_operation("store", sc::void_t(), {
            runtime.abstract_type(), runtime.abstract_type(), runtime.bitwidth()
        });

        runtime.register_operation("load", runtime.abstract_type(), {
            runtime.abstract_type(), runtime.bitwidth()
        });

        runtime.register_operation("to_tristate", sc::i8(), {
            runtime.abstract_type()
        });

        runtime.register_operation("to_bool", sc::i1(), {
            runtime.abstract_type()
        });

        runtime.register_operation("assume", sc::void_t(), {
            runtime.abstract_type(), sc::i1()
        });

        runtime.register_operation("extract", runtime.abstract_type(), {
            runtime.abstract_type(), runtime.bitwidth(), runtime.bitwidth()
        });

        runtime.register_operation("dump", sc::void_t(), {
            sc::i8p()
        });

        runtime.register_lart_api("unstash", sc::i8p(), {});
        runtime.register_lart_api("stash", sc::void_t(), { sc::i8p() });
    }

} // namespace lart::runtime
