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

#include <lava/support/base.hpp> /* iN */
#include <runtime/lart.h>

namespace __lamp
{
    using bw = __lava::bitwidth_t;

    namespace detail
    {

        // template< typename T  >
        // static dom melt_concrete( void * addr, uint64_t offset )
        // {
        //     uint8_t *i8ptr = static_cast< uint8_t * >( addr );
        //     T *moved = reinterpret_cast< T * >( i8ptr + offset );
        //     return dom::lift( *moved );
        // }

        // dom melt_concrete( void *addr, uint64_t &offset, bw size )
        // {
        //     auto correction = __vm_pointer_split( addr ).off;
        //     offset += size;
        //     switch( size ) {
        //         case 1: return melt_concrete< i8  >( addr, offset - correction - size );
        //         case 2: return melt_concrete< i16 >( addr, offset - size - correction );
        //         case 4: return melt_concrete< i32 >( addr, offset - size - correction );
        //         case 8: return melt_concrete< i64 >( addr, offset - size - correction );
        //         default:
        //             __dios_fault(
        //                     _VM_F_NotImplemented,
        //                     "peek encountered concrete value of unaligned size" );
        //     }
        // }
    } // namespace detail


    void freeze( void *val, void *addr, bw size )
    {
        // auto orig = ref( val_ );
        // auto cl = orig.clone();
        // __vm_pointer_t addr = __vm_pointer_split( addr_ ),
        //             ptr  = __vm_pointer_split( cl.disown() );

        // uint64_t current = addr.off;
        // uint64_t end_offset = addr.off + size;

        // __vm_meta_t m = { 0, 0, 0 };

        // auto advance = [ & ]() __inline {
        //     m = __vm_peek( _VM_ML_User, addr.obj, current, end_offset - current );
        // };

        // auto extract = [ & ]( uint8_t from, uint8_t to, uint32_t where ) __inline {
        //     auto full = value_ref( m );
        //     auto extracted = dom::op_extract( full, from * 8, to * 8 - 1 );
        //     auto extracted_ptr = __vm_pointer_split( extracted.disown() );
        //     // TODO: cleanup if there is already some value frozen at addr
        //     __vm_poke( _VM_ML_User, addr.obj, where, to - from, extracted_ptr.obj );
        // };

        // auto is_full_concrete = [ & ]() __inline { return m.length == 0; };

        // advance();

        // if ( current > m.offset  && !is_full_concrete() ) {
        //     extract( 0, current - m.offset, m.offset );
        // }

        // // TODO: Just look after end to see if there is some overlap.
        // while( current < end_offset && !is_full_concrete() ) {
        //     if ( current + m.length > end_offset ) {
        //         extract( end_offset - m.offset, m.length, end_offset );
        //         break;
        //     } else {
        //         // release overwritten abstract value
        //         __lamp_op_release( { __vm_pointer_make( m.value, 0 ) } );
        //     }
        //     current += (m.offset - current) + m.length;
        //     advance();
        // }

        // __vm_poke( _VM_ML_User, addr.obj, addr.off, size, ptr.obj );
        __lart_poke( addr, size, val );
    }

    void* melt( void *addr, bw size )
    {
        // __vm_pointer_t addr = __vm_pointer_split( addr_ );
        // __vm_meta_t m;
        // uint64_t end_offset = addr.off + size;
        // uint64_t current_offset = addr.off;

        // auto next = [ & ]() -> dom __inline {
        //     if ( m.length == 0 )
        //         return melt_concrete( addr_, current_offset,
        //                             end_offset - current_offset );

        //     if ( current_offset < m.offset )
        //     {
        //         auto melt_size = std::min(
        //                 m.offset - static_cast< uint64_t >( current_offset ),
        //                 end_offset - static_cast< uint64_t >( current_offset ) );
        //         return melt_concrete( addr_, current_offset, melt_size );
        //     }

        //     auto [ ext_begin, ext_end ] =
        //         [ & ]() -> std::pair< uint8_t, uint8_t > __inline {
        //             auto remaining = size - current_offset + addr.off;
        //             if ( current_offset == m.offset && m.length > remaining )
        //                 return { 0, remaining * 8 - 1 };

        //             auto base = current_offset - m.offset;
        //             if ( current_offset > m.offset && m.length > remaining )
        //                 return { base * 8, ( base + remaining ) * 8 - 1 };
        //             if ( current_offset > m.offset && m.length < remaining )
        //                 return { base * 8, ( base + m.length ) * - 1 };
        //             return { 0, m.length * 8 };
        //         } ();
        //     current_offset += m.length;

        //     auto full = value_ref( m );
        //     if ( ext_begin == 0 && ext_end == m.length * 8 )
        //         return full.clone();

        //     return dom::op_extract( full, ext_begin, ext_end );
        // };

        // auto advance = [ & ]() __inline {
        //     m = __vm_peek( _VM_ML_User, addr.obj,
        //                 current_offset,
        //                 end_offset - current_offset );
        // };

        // advance();
        // auto curr = next();
        // advance();
        // for  ( ; current_offset < end_offset; advance() )
        //     curr = dom::op_concat( next(), std::move( curr ) );
        // return curr;
        // auto peeked = __lart::rt::peek( addr, size );
        return __lart_peek( addr, size ).value;
    }
} // namespace __lamp