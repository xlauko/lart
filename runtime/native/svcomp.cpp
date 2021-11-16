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

#include "stream.hpp"

#include <execinfo.h>
 
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"

void trace_any_call(std::string_view file, std::string_view func, unsigned int line)
{
    // if (!trace.nondets)
    //  return;
    __lart::rt::file_stream out( stderr );
    out << "[lamp any] " << func << ':' << file << ':' << line << '\n';
}

#define TRACED(function) trace_any_call(file, func, line), function();

extern "C" {

    uint8_t  __lamp_any_i1(void);
    uint8_t  __lamp_any_i8(void);
    uint16_t __lamp_any_i16(void);
    uint32_t __lamp_any_i32(void);
    uint64_t __lamp_any_i64(void);
    void*    __lamp_any_ptr(void);
    
    float    __lamp_any_float(void);
    double   __lamp_any_double(void);
    
    uint8_t __lamp_any_i1_traced(const char *file, unsigned int line, const char *func)
    {
        return TRACED( __lamp_any_i1 );
    }

    uint8_t __lamp_any_i8_traced(const char *file, unsigned int line, const char *func)
    {
        return TRACED( __lamp_any_i8 );
    }
    
    uint16_t __lamp_any_i16_traced(const char *file, unsigned int line, const char *func)
    {
        return TRACED( __lamp_any_i16 );
    }
    
    uint32_t __lamp_any_i32_traced(const char *file, unsigned int line, const char *func)
    {
        return TRACED( __lamp_any_i32 );
    }
    
    uint64_t __lamp_any_i64_traced(const char *file, unsigned int line, const char *func)
    {
        return TRACED( __lamp_any_i64 );
    }
    
    float __lamp_any_float_traced(const char *file, unsigned int line, const char *func)
    {
        return TRACED( __lamp_any_float );
    }
    
    float __lamp_any_double_traced(const char *file, unsigned int line, const char *func)
    {
        return TRACED( __lamp_any_double );
    }
    
    void* __lamp_any_ptr_traced(const char *file, unsigned int line, const char *func)
    {
        return TRACED( __lamp_any_ptr );
    }
}

#pragma clang diagnostic pop
