/*
 * (c) 2021 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <memory>
#include <sys/mman.h>

namespace __lava {

    template< typename T > struct mmap_deleter
    {
        void operator()( T *ptr ) const
        {
            ptr->~T();
            munmap( ptr, sizeof( T ) );
        }
    };

    template< typename T >
    using unique_mapped_ptr = std::unique_ptr< T, mmap_deleter< T > >;

    template< typename T, typename... Args >
    unique_mapped_ptr< T > make_mmap_unique( Args &&... args )
    {
        constexpr auto PROT_RW   = PROT_READ | PROT_WRITE;
        constexpr auto MAP_ALLOC = MAP_PRIVATE | MAP_ANONYMOUS;

        if ( auto ptr = mmap( NULL, sizeof( T ), PROT_RW, MAP_ALLOC, -1, 0 ) ) {
            return unique_mapped_ptr< T >( new ( ptr ) T{ std::forward< Args >( args )... } );
        }

        std::abort();
    }

} // namespace __lava
