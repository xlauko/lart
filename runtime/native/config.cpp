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

#include "config.hpp"

#include "fault.hpp"
#include "utils.hpp"

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <string_view>

namespace __lart::rt
{
    config_t config;
    
    bool option( std::string_view option, std::string_view msg )
    {
        auto is_set = [] ( auto opt ) { return opt && strcmp( opt, "ON" ) == 0; };
        if ( auto opt = std::getenv( option.data() ); is_set( opt ) ) {
            fprintf( stderr, "[lart config] %s\n", msg.data() );
            return  true;
        }
        return false;
    }

    void load_config()
    {
        config.backtrace     = option( "LART_ERROR_BACKTRACE", "trace error backtrace" );
        config.trace_choices = option( "LART_TRACE_CHOICES", "trace choices" );
        config.ask_choices   = option( "LART_ASK_CHOICES", "ask choices" );

        if ( auto opt = std::getenv( "LART_TRACE_FILE" ); opt ) {
            fprintf( stderr, "[lart config] trace file = %s\n", opt );
            config.trace_file = std::fopen( opt, "w" );
        }
    }

    constructor void lart_setup()
    {
        load_config();

        init_fault_handler();
    }

    destructor void lart_cleanup()
    {
        if ( config.trace_file ) {
            std::fclose( config.trace_file );
        }
    }

} // namespace __lart::rt
