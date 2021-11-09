/* TAGS: sym c++ */
/* VERIFY_OPTS: --symbolic */
/* CC_OPTS: */

// V: v.O0 CC_OPT: -O0 TAGS: min
// V: v.O1 CC_OPT: -O1
// V: v.O2 CC_OPT: -O2 TAGS: min
// V: v.Os CC_OPT: -Os
#include <lamp.h>
#include <stdint.h>

#include <cstdint>
#include <assert.h>

int main() {
    uint32_t array[ 2 ] = { 0 };
    uint32_t x = __lamp_any_i32();
    array[ 0 ] = x;

    uint64_t val = *reinterpret_cast< uint64_t * >( array );
    assert( array[ 0 ] == val );
    assert( array[ 1 ] == val >> 32 );
}
