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
#include <cstdio>
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

        template< typename stream >
        friend auto operator<<(stream &os, const report_payload &report) noexcept -> decltype( os << "" )
        {
            return os << report.what();
        }

    private:
        std::string_view _report;
    };


    struct source_location
    {
        static constexpr source_location current(
            std::string_view file = __builtin_FILE(),
            std::string_view function = __builtin_FUNCTION(),
            unsigned line = __builtin_LINE()
        ) noexcept;

        constexpr source_location( std::string_view file
                                 , std::string_view func
                                 , unsigned line) noexcept
            :  _file(file), _func(func), _line(line)
        {}

        constexpr std::string_view file() const noexcept { return _file; }
        constexpr std::string_view function() const noexcept { return _func; }

        constexpr unsigned line() const noexcept { return _line; }

        template< typename stream >
        friend auto operator<<( stream &os, const source_location &loc ) noexcept -> decltype( os << "" )
        {
            std::array<char, 33> line;
            std::sprintf(line.data(), "%d", loc.line());
            return os << loc.file() << ":" << loc.function() << ":" << line.data();
        }

    private:
        std::string_view _file;
        std::string_view _func;
        unsigned _line;
        unsigned _column;
    };

    enum class fault_type { assert_failed };

    struct fault_event
    {
        fault_type type;
        source_location location;
        report_payload report;
    };

    [[noreturn]] void fault( const fault_event& event ) noexcept;

    void init_fault_handler() noexcept;
} // namespace __lart::rt
