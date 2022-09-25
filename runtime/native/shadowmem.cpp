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

#include <cstdint>
#include <cassert>

#include <memory>
#include <vector>
#include <string_view>
#include <unordered_map>

#include "shadowmem.hpp"

namespace __lart::rt
{
    std::unordered_map< std::uintptr_t, shadow_label_t > __lart_shadows;

    // TODO enable mark & sweep (clean up shadow info map)
    std::vector< shadow_label_info > __lart_shadow_info;

    shadow_label_t create_shadow_label(void *meta) {
        __lart_shadow_info.push_back(shadow_label_info{ .userdata = meta });
        return __lart_shadow_info.size();
    }

    void set_shadow_label(shadow_label_t label, void *addr, size_t size) {
        for (auto offset = 0; offset < size; ++offset) {
            __lart_shadows[uintptr_t(addr) + offset] = label;
        }
    }

    // TODO generator?
    // TODO use size?
    shadow_label_t read_shadow_label(const void *addr, size_t size) {
        if (auto label = __lart_shadows.find(uintptr_t(addr)); label != __lart_shadows.end()) {
            return label->second;
        }
        return 0; // TODO
    }

    shadow_label_info get_shadow_label_info(shadow_label_t label) {
        // assert(label <= __lart_shadow_info.size());
        return __lart_shadow_info[label - 1];
    }

    // shadow_label_t has_label_with_desc(shadow_label_t label, const char *desc) {
    //     return 0; // TODO
    // }

    // shadow_label_t get_shadow_label(long data) {
    //     return 0; // TODO
    // }

    // bool has_shadow_label(shadow_label_t label, shadow_label_t elem) {
    //     return false; // TODO
    // }



} // namespace __lart::rt
