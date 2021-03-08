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

#include <cstdio>
#include <cstdlib>

namespace __lart::rt
{
    struct trace_t
    {
        struct choices_t {

            ~choices_t() { free( trace ); }

            inline void push( int choice )
            {
                size += 1;
                trace = static_cast< int* >(
                    realloc( trace, size * sizeof( int ) )
                );
                trace[size - 1] = choice;
            }

            inline auto begin() { return trace; }
            inline auto end() { return trace + size; }

            unsigned size = 0;
            int* trace = nullptr;
        };

        ~trace_t()
        {
            if ( choices.trace ) {
                for ( auto c : choices )
                    fprintf( stderr, "[lart-choice] %d\n", c );
                fprintf( stderr, "----------------\n" );
            }
        }

        inline void push_choice( int c ) { choices.push( c ); }

        choices_t choices;
    };

    extern trace_t trace;
} // namespace __lart::rt
