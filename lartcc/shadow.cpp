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

#include <cc/shadow.hpp>

namespace lart
{
    std::string to_string(shadow_op_kind kind) {
        switch (kind) {
            case shadow_op_kind::source:
                return "source";
            case shadow_op_kind::memory:
                return "memory";
            case shadow_op_kind::forward:
                return "forward";
            case shadow_op_kind::store:
                return "store";
            case shadow_op_kind::load:
                return "load";
        }

        __builtin_unreachable();
    }

    shadow_op_kind operation_kind( llvm::Value *val ) {
        if ( llvm::isa< llvm::AllocaInst >(val) )
            return shadow_op_kind::memory;
        else if ( llvm::isa< llvm::StoreInst >(val) )
            return shadow_op_kind::store;
        else if ( llvm::isa< llvm::LoadInst >(val) )
            return shadow_op_kind::load;
        return shadow_op_kind::forward;
    }

    sc::generator< shadow_operation > shadow::toprocess()
    {
        for ( auto &[val, type] : types ) {
            co_yield { val, operation_kind(val) };
        }
    }

    std::optional< ir::intrinsic > shadow::process( shadow_operation o )
    {
        spdlog::debug("[shadow] make {} op: {}",
            to_string(o.kind),
            sc::fmt::llvm_to_string(o.value)
        );

        switch (o.kind) {
            case shadow_op_kind::source:
                return process_source(o);
            case shadow_op_kind::memory:
                return process_memory(o);
            case shadow_op_kind::forward:
                return process_forward(o);
            case shadow_op_kind::store:
                return process_store(o);
            case shadow_op_kind::load:
                return process_load(o);
        }
        return std::nullopt;
    }

    std::optional< ir::intrinsic > shadow::process_source( shadow_operation /* op */ ) {
        return std::nullopt;
    }

    std::optional< ir::intrinsic > shadow::process_memory( shadow_operation /* op */ ) {
        return std::nullopt;
    }

    std::optional< ir::intrinsic > shadow::process_forward( shadow_operation /* op */ ) {
        return std::nullopt;
    }

    std::optional< ir::intrinsic > shadow::process_load( shadow_operation /* op */ ) {
        return std::nullopt;
    }

    std::optional< ir::intrinsic > shadow::process_store( shadow_operation /* op */ ) {
        return std::nullopt;
    }


} // namespace lart
