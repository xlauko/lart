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
#include <unordered_map>

namespace __lart::rt
{
    // TODO interval map?
    // maps bytes to shadow identifiers
    std::unordered_map< std::uintptr_t, shadow_label_t > shadow;

    // TODO dense map
    // maps shadow identifier to user metadata
    std::unordered_map< shadow_label_t, shadow_label_info > shadow_info;

    shadow_label_t create_shadow_label(shadow_label_info info) {
        shadow_label_t label = shadow_info.size() + 1;
        shadow_info.emplace(label, info);
        return label;
    }

    void set_shadow_label(shadow_label_t label, void *addr, size_t size) {
        for (auto offset = 0; offset < size; ++offset) {
            shadow[uintptr_t(addr) + offset] = label;
        }
    }

    void poke(void *addr, size_t bytes, void *value)
    {
        auto label = create_shadow_label(shadow_label_info{
            .value = value, .origin = addr, .bytes = bytes
        });
        set_shadow_label( label, addr, bytes );
    }

    // TODO use size
    sc::generator< shadow_label_t > read_shadow_label(const void *addr, size_t size) {
        if (auto label = shadow.find(uintptr_t(addr)); label != shadow.end()) {
            co_yield label->second;
        } else {
            fprintf( stderr, "[lart fault] missing shadow\n" );
        }
    }

    shadow_label_info get_shadow_label_info(shadow_label_t label) {
        return shadow_info[label];
    }

    sc::generator< shadow_label_info > peek( const void *addr )
    {
        for (auto label : read_shadow_label( addr, 1 )) {
            if ( !shadow_info.count(label) ) {
                fprintf( stderr, "[lart fault] missing shadow info\n" );
            } else {
                co_yield get_shadow_label_info( label );
            }
        }
    }

    bool test_taint( void *addr )
    {
        return shadow.find( uintptr_t(addr) ) != shadow.end();
    }

} // namespace __lart::rt
