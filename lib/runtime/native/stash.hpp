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
    using stash_register_t = std::uint64_t;

    extern thread_local stash_register_t stash_register;

    template< typename T > void stash( T value )
    {
        if constexpr ( std::is_pointer_v< T > )
            stash_register = reinterpret_cast< std::uintptr_t >( value );
        else
            stash_register = static_cast< std::uint64_t >( value );
    }

    template< typename T > T unstash()
    {
        auto ret = [] {
            if constexpr ( std::is_pointer_v< T > )
                return reinterpret_cast< T >( stash_register );
            else
                return static_cast< T >( stash_register );
        }();

        stash_register = 0; // reset stash_register
        return ret;
    }

} // namespace __lart::rt
