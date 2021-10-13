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
#include "state.hpp"
#include "fault.hpp"

#include "stream.hpp"

#include <execinfo.h>

#include <csignal>
#include <cstdio>

#include "utils.hpp"

namespace __lart::rt
{
    constexpr fixed_report::fixed_report( std::string_view report ) noexcept
        : _report{}, _report_size{0}
    {
        auto end = [&] {
            const bool should_truncate = report.size() > report_reserve_size;
            if ( should_truncate ) {
                auto &trunc = truncation_message;
                auto clip = report_reserve_size - trunc.size();
                auto end = std::copy_n( report.begin(), clip, _report.begin() );
                return std::copy( trunc.begin(), trunc.end(), end );
            }
            return std::copy( report.begin(), report.end(), _report.begin() );
        } ();
        
        _report_size = std::distance( _report.begin(), end );
    }

    static_assert( fixed_report( "short report" ).what() == "short report" );

    constexpr source_location source_location::current(
        std::string_view file,
        std::string_view function,
        unsigned line, unsigned column
    ) noexcept
    {
        return source_location( file, function, line, column );
    }

    struct fault_handler
    {
        void handle( const fault_event &event ) const noexcept
        {
            switch (event.type)
            {
                case fault_type::assert_failed: handle_assert_failed(event);
            }
        }

        void handle_assert_failed(const fault_event &event) const noexcept
        {
            file_stream out( stderr );
            out << "[lart fault] assertion " << event.report << " failed at: " 
                << event.location << "\n";
            
            if ( config.backtrace ) {
                print_backtrace(out);
            }
        }

        void print_backtrace(file_stream &out) const noexcept
        {
            constexpr size_t size = 1024;
            void *callstack[size];
            auto frames = backtrace(callstack, size);
            auto symbols = backtrace_symbols(callstack, frames);

            std::string_view header = "[lart backtrace]";
            if (!symbols) {
                out << header << " empty\n";
                return;
            }

            for (int i = 0; i < frames; ++i) {
                auto symbol = symbols[i];
                if (!symbol || ignore_symbol(symbol)) 
                    continue;

                out << header << ' ' <<  format_symbol(symbol) << '\n';
            }

            std::free(symbols);
        }

        bool ignore_symbol(std::string_view symbol) const
        {
            return symbol.find("__lart") != std::string_view::npos;
        }

        std::string_view format_symbol(std::string_view symbol) const
        {
            auto from = symbol.find('(');
            auto to = symbol.rfind('+');

            constexpr auto npos = std::string_view::npos;
            
            if (from != npos && to != npos) {
                symbol = symbol.substr(from + 1, to - from - 1);
            }

            if (symbol.starts_with("dfs$"))
                symbol.remove_prefix(sizeof("dfs$") - 1);

            return symbol.empty() ? "unknown" : symbol;
        }
    };

    fault_handler handler;
    
    [[noreturn]] void fault( const fault_event &event ) noexcept
    {
        handler.handle(event);
        std::exit(EXIT_FAILURE);
    }

    void init_fault_handler() noexcept {}
} // namespace __lart::rt
