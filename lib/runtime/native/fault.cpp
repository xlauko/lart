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

#include "fault.hpp"

#include <csignal>

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

    void init_fault_handler()
    {
        printf( "init fault handler\n" );
    }
} // namespace __lart::rt
