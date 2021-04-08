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

#include <lamp/adaptor/pa.hpp>
#include <lava/term.hpp>

namespace __lamp
{
    using pointers = adaptor::pointers< __lava::term >;
    using meta_domain = semilattice< pointers >;

} // namespace __lamp

#include "wrapper.hpp"

namespace __lava
{
    void * __pointers_state;
}

[[gnu::constructor]] void __lamp_pointers_init()
{
    using namespace __lava;
    using state_t = __lamp::pointers::pa::state_t;
    __pointers_state = malloc( sizeof( state_t ) );
    new ( __pointers_state ) state_t;
}

[[gnu::destructor]] void __lamp_pointers_fini()
{
    using namespace __lava;
    using state_t = __lamp::pointers::pa::state_t;

    auto state = static_cast< state_t* >( __pointers_state );
    state->map.clear();
    free( state );
}


extern "C" void* __lamp_lift_objid( void* p )
{
    return lift( [] ( auto ptr ) {
        return __lamp::meta_domain( __lamp::pointers::lift_objid( ptr ) );
    }, p );
}