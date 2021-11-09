/* TAGS: error sym c++ */
/* VERIFY_OPTS: --symbolic */
/* CC_OPTS: */

// V: v.O0 CC_OPT: -O0 TAGS: error min
#include <lamp.h>
#include <stdint.h>

#include <cstdint>
#include <assert.h>

__attribute__(( __noinline__)) int id ( int a ) { return a; }

int main() {
    uint64_t array[ 4 ];
    for ( int i = 0; i < 4; ++i )
        array[ i ] = __lamp_any_i64();
    if ( array[ 4 ] ) /* ERROR */ // this is undefined behaviour, it is optimized out with -O2
        return id( 0 );
    return id( 1 );
}
