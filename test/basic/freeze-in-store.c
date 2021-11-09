/* TAGS: sym c++ min */
/* VERIFY_OPTS: --symbolic */
/* CC_OPTS: */

// V: v.O0 CC_OPT: -O0
// V: v.O1 CC_OPT: -O1
// V: v.O2 CC_OPT: -O2
// V: v.Os CC_OPT: -Os
#include <lamp.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

int x = 0;

int *make_abstract( uintptr_t local )
{
    if ( x ) {
        local += __lamp_any_i32();
        return ( int * )local;
    }
    return ( int * )local;
}

__attribute__((__noinline__))
int get() { return x; }

int main() {
    int p_local[12];
    int *local = make_abstract( uintptr_t( p_local ) );
    local[2] = 21;
    local[3] = get();
    assert( local[3] == x );
}
