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
    static unsigned depth = 0;

    void check_terminate(int status = 0)
    {
        if (config->error_found) {
            std::exit(EXIT_SUCCESS);
        }

        auto bound = config->choose_bound;
        if ( bound && depth >= bound ) {
            fprintf(stderr, "[lart status] bounded exit\n");
            std::exit(EXIT_SUCCESS);
        }
    }

    int fork_choose_dec( int count )
    {
        if ( count == 1 )
            return 0;
        auto pid = fork();
        if ( pid == 0 )
            return count - 1;
        int status;
        pid_t done = wait(&status);

        check_terminate( status ); 
        return choose( count - 1 );
    }

    int fork_choose_inc( int count )
    {
        for (int i = 0; i < count - 1; i++) {
            auto pid = fork();
            if ( pid == 0 )
                return count - 1;
            int status;
            pid_t done = wait(&status);
            check_terminate( status );
        }

        return count - 1;
    }

    int choose( int count )
    {
        int result = 0;
        if ( config->ask_choices ) {
            std::scanf( "%d", &result );
        // TODO: remove unnecessary condition
        } else if ( config->choose_increasing ) {
            result = fork_choose_inc( count );
        } else {
            result = fork_choose_dec( count );
        }

        if ( config->trace_choices )
            trace.push_choice( result );

        ++depth;
        return result;
    }

} // namespace __lart::rt
