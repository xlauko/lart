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

namespace __lart::rt
{   
    struct file_stream
    {
        explicit file_stream(std::FILE *file = stderr) : _file(file) {}
        ~file_stream() { std::fflush(_file); }

        file_stream& operator<<(std::string_view str) noexcept
        {
            std::fwrite( str.data(), sizeof(char), str.size(), _file );
            return *this;
        }

        file_stream& operator<<(char c) noexcept
        {
            std::fprintf( _file, "%c", c );
            return *this;
        }

        file_stream& operator<<(unsigned int ui) noexcept
        {
            std::fprintf( _file, "%u", ui );
            return *this;
        }
        
        file_stream& operator<<(int si) noexcept
        {
            std::fprintf( _file, "%d", si );
            return *this;
        }

        file_stream& operator<<(unsigned short us) noexcept
        {
            std::fprintf( _file, "%hu", us );
            return *this;
        }
        
        file_stream& operator<<(short ss) noexcept
        {
            std::fprintf( _file, "%hd", ss );
            return *this;
        }

        file_stream& operator<<(unsigned long ul) noexcept
        {
            std::fprintf( _file, "%lu", ul );
            return *this;
        }
        
        file_stream& operator<<(long sl) noexcept
        {
            std::fprintf( _file, "%ld", sl );
            return *this;
        }

        std::FILE* stream() { return _file; }
    
    protected:
        std::FILE *_file;
    };

} // namespace __lart::rt