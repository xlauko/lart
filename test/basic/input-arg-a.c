/* TAGS: sym c++ */
/* VERIFY_OPTS: --symbolic */
/* CC_OPTS: */

// V: v.O0 CC_OPT: -O0 TAGS: min
// V: v.O1 CC_OPT: -O1 TAGS: min
#include <lamp.h>
#include <stdint.h>
#include <assert.h>

#define INLINE __attribute__((__noinline__))

INLINE int choose( int a, int b ) { return ( __lamp_any_i8() ) ? a : b; }

int main() {
    int one = __lamp_any_i32();
    int chosen = choose( one, one );
    assert( chosen == one );
}

#undef INLINE
