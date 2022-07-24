/*
 * (c) 2021 Henrich Lauko <xlauko@mail.muni.cz>
 * (c) 2020 Petr Ročkai <code@fixp.eu>
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

#include <cstdint>
#include <memory>
#include <limits>

#include <lava/support/base.hpp> /* domain_ref, construct_shared */

namespace __lamp
{
    using bitwidth_t = uint8_t;
    using tag_t = uint8_t;

    static constexpr tag_t invalid_tag = std::numeric_limits< tag_t >::max();

    using construct_shared_t = __lava::construct_shared_t;
    static constexpr auto construct_shared = __lava::construct_shared;

    template< typename type >
    struct wrapped
    {
        using value_type = type;

        template< typename ...args >
        wrapped( args &&...a ) : _value( std::forward< args >(a)... ) {}

        value_type&       value()       { return _value; }
        const value_type& value() const { return _value; }

    private:
        value_type _value;
    };

    template< typename type >
    struct /* [[gnu::packed]] */ tagged
    {
        using value_type = type;

        template< typename ...args >
        tagged( args &&...a ) : _value( std::forward< args >(a)... ) {}

        value_type&       value()       { return _value; }
        const value_type& value() const { return _value; }

        tag_t &tag()        { return _tag; }
        tag_t  tag()  const { return _tag; }

    private:
        uint8_t _tag = invalid_tag;
        value_type _value;
    };

    struct empty {};

    template< typename kind >
    struct storage : kind
    {
        using value_type = typename kind::value_type;

        template< typename ...args >
        storage( args &&...a ) : kind( std::forward< args >(a)... ) {}

        void* operator new( size_t s ) { return std::malloc(s); }
        void  operator delete( void* m ) { std::free(m); }
    };

    template< typename storage >
    struct pointer
    {
        using value_type = typename storage::value_type;

        template< typename ...args >
        pointer( args &&...a )
            : _storage( new storage( std::forward< args >(a)... ) )
        {}

        pointer( void *ptr, construct_shared_t )
            : _storage( static_cast< storage* >( ptr ) )
        {}

        const value_type &get() const { return _storage->value(); }
        value_type       &get()       { return _storage->value(); }

        const storage &store() const { return *_storage; }
        storage       &store()       { return *_storage; }

        const value_type *operator->() const { return &get(); }
        value_type       *operator->()       { return &get(); }

        void *unsafe_ptr() const { return _storage.get(); }

        void *disown() { return _storage.release(); }

    private:
        std::unique_ptr< storage > _storage;
    };

    template< typename data >
    using wrapped_storage = pointer< storage< wrapped< data > > >;

    template< typename data >
    using tagged_storage = pointer< storage< tagged< data > > >;

    using any_tagged_storage = pointer< storage< tagged< empty > > >;

} // namespace __lamp

