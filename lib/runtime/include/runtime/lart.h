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

#define __annotate( x )   __attribute__(( __annotate__( #x ) ))
#define __inline          __attribute__(( __always_inline__ ))
#define __noinline        __attribute__(( __noinline__ ))
#define __flatten         __attribute__(( __flatten__ ))

#define __ignore_return   __annotate( lart.transform.ignore.ret )
#define __ignore_args     __annotate( lart.transform.ignore.arg )
#define __noalias_return  __annotate( lart.noalias.ret )
#define __noalias_args    __annotate( lart.noalias.arg )
#define __noalias         __noalias_return __noalias_args
#define __unused          __attribute__((unused))
#define __used            __attribute__((used))
#define __export          __attribute__((weak))
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
    __export void __lart_stash( void* abstract ) __lart_stub;

    __export void* __lart_unstash()  __lart_stub;

    __export bool __lart_test_taint( uint8_t byte ) __lart_stub;

    __export void __lart_set_taint( void *value, unsigned bytes )  __lart_stub;

    __export int __lart_choose( int count ) __lart_stub;

    __export void __lart_cancel() __lart_stub;

    __export void* __lart_melt( void *addr, uint32_t bw ) __lart_stub;
    __export void __lart_freeze( void *value, void *addr, uint32_t bw ) __lart_stub;

    __export void __lart_assert_fail( const char* file, const char *func, unsigned line, unsigned col );

    #define assert( x ) { if(!(x)) \
        __lart_assert_fail(__builtin_FILE(), __builtin_FUNCTION(), __builtin_LINE(), __builtin_COLUMN()); \
    }

#ifdef __cplusplus
}
#endif

__lart_pop_diagnostic
