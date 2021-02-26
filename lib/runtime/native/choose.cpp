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

#include "choose.hpp"
#include "utils.hpp"
#include "array.hpp"

#include <cstdlib>
#include <cstring>
#include <cstdio>

namespace __lart::rt
{
    __lart::array< int > lart_path;

    int choose( int count )
    {
        static int choice = 0; // TODO make thread local?

        int result = 0;
        if ( choice < lart_path.size() )
            result = lart_path[choice];
        fprintf(stderr, "[lart-choice]: %d\n", result );
        return result;
    }

    constructor void choose_config() noexcept
    {
        char *path = getenv("LART_PATH");
        char *choice= strtok( path, "-" );
        while( choice != NULL ) {
            lart_path.push_back( atoi( choice ) );
            choice = strtok( NULL, "-" );
        }
    }

} // namespace __lart::rt
