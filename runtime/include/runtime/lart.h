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

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "shadowmeta.h"

#define __lart_annotate( x )  __attribute__(( __annotate__( #x ) ))
#define __lart_inline         __attribute__(( __always_inline__ ))
#define __lart_noinline       __attribute__(( __noinline__ ))
#define __lart_flatten        __attribute__(( __flatten__ ))

#define __lart_ignore_return   __lart_annotate( lart.transform.ignore.ret )
#define __lart_ignore_args     __lart_annotate( lart.transform.ignore.arg )
#define __lart_noalias_return  __lart_annotate( lart.noalias.ret )
#define __lart_noalias_args    __lart_annotate( lart.noalias.arg )
#define __lart_noalias         __lart_noalias_return __lart_noalias_args
#define __lart_unused          __attribute__((unused))
#define __lart_used            __attribute__((used))
#define __lart_export          __attribute__((weak))
#define __lart_stub       { __builtin_unreachable(); }

#define __lart_ignore_diagnostic \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \

#define __lart_pop_diagnostic \
    _Pragma("GCC diagnostic pop") \

__lart_ignore_diagnostic

#ifdef __cplusplus
extern "C" {
#endif
   /* Abstract versions of arguments and return values are passed in a 'stash', a
    * global (per-thread) location. On 'unstash', we clear the stash for two
    * reasons: 1) it flags up otherwise hard-to-pin bugs where mismatched
    * stash/unstash reads an old value of the stash, and 2) when verification
    * encounters a possible diamond (the explicit content of the state is the
    * same, e.g.  after an if/else) and we leave old stashes around, it might so
    * happen that the stashed values in the candidate states are of different
    * types, leading to a type mismatch in the comparison.
    *
    * TODO The second problem might have to be tackled in the verification
    * algorithm anyway, since there might be other ways to trigger the same
    * problem. */
    __lart_export void __lart_stash( void* abstract ) __lart_stub

    __lart_export void* __lart_unstash()  __lart_stub

    __lart_export bool __lart_test_taint( uint8_t byte ) __lart_stub

    __lart_export void __lart_set_taint( void *value, unsigned bytes )  __lart_stub

    __lart_export int __lart_choose( int count ) __lart_stub

    __lart_export void __lart_cancel() __lart_stub

    __lart_export void __lart_poke( void *addr, size_t bytes, void* value );

    __lart_export struct __lart_shadow_meta *__lart_peek( const void *addr );

#ifdef __cplusplus
}
#endif

__lart_pop_diagnostic
