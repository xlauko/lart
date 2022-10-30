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

#include <shadow.hpp>

#include <sc/generator.hpp>

#include <cstddef>
#include <cstdlib>

#include <memory>
#include <vector>
#include <unordered_map>

namespace __lart::rt
{
    // TODO interval map?
    // maps bytes to shadow identifiers
    std::unordered_map< std::uintptr_t, shadow_label_t > shadow;

    // TODO dense map
    // maps shadow identifier to user metadata
    std::unordered_map< shadow_label_t, shadow_label_info > shadow_info;

    struct frame_t { std::vector< std::uintptr_t > addrs; };

    std::vector< frame_t > frames;

    std::unordered_map< std::uintptr_t, frame_t* > allocated;

    frame_t& current_frame() { return frames.back(); }

} // namespace __lart::rt

extern "C" {
    void __lart_setup_shadow();

    void __lart_entry_frame()
    {
        __lart::rt::frames.push_back({});
    }

    void __lart_exit_frame()
    {
        for (auto addr : __lart::rt::current_frame().addrs) {
            __lart::rt::shadow.erase(addr);
            __lart::rt::allocated.erase(addr);
        }
        __lart::rt::frames.pop_back();
    }

}

namespace __lart::rt {

    [[gnu::constructor]] void setup_shadow() {
        __lart_entry_frame(); // setup global frame
    }

    shadow_label_t create_shadow_label(shadow_label_info info) {
        shadow_label_t label = shadow_info.size() + 1;

        shadow_info.emplace(label, info);
        return label;
    }

    void set_shadow_label(shadow_label_t label, void *addr, size_t size) {
        for (auto offset = 0; offset < size; ++offset) {
            // TODO intervals
            auto byte = uintptr_t(addr) + offset;
            if (!allocated.count(byte)) {
                current_frame().addrs.push_back(byte);
                allocated[byte] = &current_frame();
            }
            shadow[byte] = label;
        }
    }

    void erase_shadow(void *addr, size_t size) {
        for (auto offset = 0; offset < size; ++offset) {
            shadow.erase(uintptr_t(addr) + offset);
        }
    }

    void poke(void *addr, size_t bytes, void *value) {
        if (value) {
            auto label = create_shadow_label(shadow_label_info{
                .value = value, .origin = addr, .bytes = bytes
            });
            set_shadow_label( label, addr, bytes );
        } else {
            // erase shadow when storing nullptr value
            erase_shadow( addr, bytes );
        }
    }

    shadow_label_info get_shadow_label_info(shadow_label_t label) {
        return shadow_info[label];
    }

    sc::generator< shadow_label_t > read_shadow_label(const void *addr, size_t bytes) {
        for (auto offset = 0; offset < bytes;) {
            auto byte = uintptr_t(addr) + offset;
            if (auto label = shadow.find(byte); label != shadow.end()) {
                co_yield label->second;
                offset += get_shadow_label_info( label->second ).bytes;
            } else {
                co_yield shadow_label_t{ 0 };
                offset += 1;
            }
        }
    }

    sc::generator< shadow_label_info > peek(const void *addr, size_t bytes)
    {
        for (auto label : read_shadow_label( addr, bytes )) {
            if ( label ) {
                co_yield get_shadow_label_info( label );
            } else {
                co_yield shadow_label_info{ .value = nullptr, .origin = nullptr, .bytes = 1 };
            }
        }
    }

    bool test_taint(void *addr, size_t bytes)
    {
        // TODO use interval map
        for (auto offset = 0; offset < bytes; ++offset) {
            if (shadow.find( uintptr_t(addr) + offset ) != shadow.end()) {
                return true;
            }
        }

        return false;
    }

} // namespace __lart::rt
