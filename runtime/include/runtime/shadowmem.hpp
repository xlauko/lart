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

#pragma once

#include <cstdint>
#include <string_view>

namespace __lart::rt
{
    using shadow_label_t = std::uint64_t;

    struct shadow_label_info {
        // Fields for union labels, set to 0 for base labels.
        shadow_label_t l1;
        shadow_label_t l2;

        // Fields for base labels.
        const char *desc;
        void *userdata;
    };

    shadow_label_t create_shadow_label( std::string_view name, void *meta) {
        return 0; // TODO
    }

    void set_shadow_label(shadow_label_t label, void *addr, size_t size) {}

    shadow_label_t read_shadow_label(const void *addr, size_t size) {
        return 0; // TODO
    }

    const struct shadow_label_info* get_shadow_label_info(shadow_label_t label) {
        return nullptr;
    }

    shadow_label_t has_label_with_desc(shadow_label_t label, const char *desc) {
        return 0; // TODO
    }

    shadow_label_t get_shadow_label(long data) {
        return 0; // TODO
    }

    bool has_shadow_label(shadow_label_t label, shadow_label_t elem) {
        return false; // TODO
    }



} // namespace __lart::rt
