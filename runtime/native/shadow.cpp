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
#include <sanitizer/dfsan_interface.h>

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

    void poke( void *addr, size_t size, void *value )
    {
        auto meta = make_meta( addr, size, value );
        auto shadow = dfsan_create_label( "shadow", meta );
        dfsan_set_label( shadow, addr, size );
    }

    shadow_meta *peek( const void *addr )
    {
        auto meta        = dfsan_read_label( addr, 1 );
        const auto *info = dfsan_get_label_info( meta );
        return static_cast< shadow_meta* >( info->userdata );
    }

    shadow_meta *peek( const void *addr, std::size_t size )
    {
        return peek( addr );
    }

} // namespace __lart::rt
