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

#pragma once

#include <string_view>
#include <array>

namespace __lart::rt
{
    struct fixed_report
    {
        static constexpr size_t report_reserve_size = 30;
        static constexpr std::string_view truncation_message = "... (error report truncated)";

        using storage = std::array< char, report_reserve_size >;

        constexpr fixed_report( std::string_view report ) noexcept;
        
        [[nodiscard]] constexpr inline std::string_view what() const noexcept
        {
            return std::string_view( _report.data(), _report_size );
        }

    private:
        storage _report;
        size_t  _report_size;
    };

    struct report_payload
    {
        explicit constexpr report_payload( std::string_view report ) noexcept
            : _report(report) {}

        explicit constexpr report_payload( fixed_report report ) noexcept
            : _report(report.what()) {}
        
        [[nodiscard]] constexpr inline std::string_view what() const noexcept
        {
            return _report;
        }

    private:
        std::string_view _report;
    };

    void init_fault_handler();
} // namespace __lart::rt
