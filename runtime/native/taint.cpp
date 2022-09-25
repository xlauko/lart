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

#include "taint.hpp"

#include "utils.hpp"

#include <cstdio>

namespace __lart::rt
{
    // shadow_label_t taint;

    // constructor void init_taint()
    // {
    //     taint = create_shadow_label( "taint", nullptr );
    // }

    // void make_tainted( void *value, unsigned bytes )
    // {
    //     set_shadow_label( taint, value, bytes );
    // }

    // bool is_tainted( void *addr )
    // {
    //     return false;
    //     // auto label = get_shadow_label( static_cast< std::int64_t >( value ) );
    //     // return label && (has_shadow_label( label, taint ) ||
    //     //        has_label_with_desc( label, "shadow" ));
    // }

} // namespace __lart::rt
