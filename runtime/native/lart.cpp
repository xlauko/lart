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

#include "fault.hpp"
#include "stash.hpp"
#include "taint.hpp"
#include "shadow.hpp"
#include "choose.hpp"

#include "shadowmeta.h"

#include <cstdlib>

extern "C"
{

    void * __lart_unstash()
    {
        return __lart::rt::unstash< void * >();
    }

    void __lart_stash( void *val )
    {
        __lart::rt::stash( val );
    }

    bool __lart_test_taint( uint8_t byte )
    {
        return __lart::rt::is_tainted( byte );
    }

    void __lart_set_taint( void *value, unsigned bytes )
    {
        __lart::rt::make_tainted( value, bytes );
    }

    int __lart_choose(int count)
    {
        return __lart::rt::choose(count);
    }

    void __lart_cancel()
    {
        std::exit( EXIT_SUCCESS );
    }

    void __lart_poke( void *addr, size_t bytes, void* value )
    {
        __lart::rt::poke( addr, bytes, value );
    }

    __lart_shadow_meta *__lart_peek( const void *addr )
    {
        return __lart::rt::peek( addr );
    }
    
    void __assert_fail(const char *assertion, const char *file, unsigned int line, const char *func)
    {
        using fault_event = __lart::rt::fault_event;
        using fault_type  = __lart::rt::fault_type;

        auto loc = __lart::rt::source_location(file, func, line, 0);
        auto msg = __lart::rt::report_payload(assertion);
        __lart::rt::fault(fault_event{fault_type::assert_failed, loc, msg});
    }
}
