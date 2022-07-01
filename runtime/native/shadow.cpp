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

#include "shadow.hpp"

#include <cstddef>
#include <cstdlib>

namespace __lart::rt
{
    void* make_meta( void *addr, size_t bytes, void *value )
    {
        auto meta = (shadow_meta*)malloc( sizeof(shadow_meta) );
        meta->value = value;
        meta->origin = addr;
        meta->bytes = bytes;
        return meta;
    }


    void poke( void *addr, size_t bytes, void *value )
    {
        auto meta = make_meta( addr, bytes, value );
        auto shadow = create_shadow_label( "shadow", meta );
        set_shadow_label( shadow, addr, bytes );
    }

    shadow_meta *peek( const void *addr )
    {
        auto meta        = read_shadow_label( addr, 1 );
        const auto *info = get_shadow_label_info( meta );
        return static_cast< shadow_meta* >( info->userdata );
        return nullptr;
    }

} // namespace __lart::rt
