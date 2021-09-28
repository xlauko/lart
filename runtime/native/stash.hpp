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

#pragma once

#include <cstdint>
#include <type_traits>

namespace __lart::rt
{
    constexpr std::size_t stash_stack_size = 256;
    
    using stash_stack_value_t = std::uint64_t;
    
    extern thread_local std::uint8_t stash_stack_top;
    extern thread_local stash_stack_value_t stash_stack[];

    template< typename T > void stash( T value )
    {
        auto &place = stash_stack[ stash_stack_top++ ];
        if constexpr ( std::is_pointer_v< T > )
            place = reinterpret_cast< std::uintptr_t >( value );
        else
            place = static_cast< std::uint64_t >( value );
    }

    template< typename T > T unstash()
    {
        if constexpr ( std::is_pointer_v< T > )
            return reinterpret_cast< T >( stash_stack[ --stash_stack_top ] );
        else
            return static_cast< T >( stash_stack[ --stash_stack_top ] );
    }

} // namespace __lart::rt
