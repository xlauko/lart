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
#include "config.hpp"
#include "trace.hpp"

#include <unistd.h>
#include <sys/wait.h>

namespace __lart::rt
{
    int fork_choose( int count )
    {
        if ( count == 1 )
            return 0;
        auto pid = fork();
        if ( pid == 0 )
            return count - 1;
        int status;
        pid_t done = wait(&status);
        return fork_choose( count - 1 );
    }

    int choose( int count )
    {
        int result;
        if ( config.ask_choices ) {
            std::scanf( "%d", &result );
        } else {
            result = fork_choose( count );
        }
        if ( config.trace_choices )
            trace.push_choice( result );
        return result;
    }

} // namespace __lart::rt
