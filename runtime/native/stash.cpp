/*
 * (c) 2020 Henrich Lauko <xlauko@mail.muni.cz>
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

#include "stash.hpp"

namespace __lart::rt
{
    thread_local std::uint8_t stash_stack_top = 0;
    thread_local stash_stack_value_t stash_stack[abstract_stack_size] = {};

    thread_local std::uint8_t taint_stack_top = 0;
    thread_local bool taint_stack[abstract_stack_size] = {};

    void stash_taint( bool taint )
    {
        taint_stack[ taint_stack_top++ ] = taint;
    }

    bool unstash_taint()
    {
        return taint_stack[ --taint_stack_top ];
    }
} // namespace __lart::rt
