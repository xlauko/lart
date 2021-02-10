#pragma once

#include <stdint.h>
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
    void __lart_stash( void * );
    void * __lart_unstash();


    _Bool __lart_test_taint( uint8_t byte );

    void __lart_set_taint( void *value, unsigned bytes );

    int __lart_choose( int count );

    void __lart_cancel();
#ifdef __cplusplus
}
#endif
