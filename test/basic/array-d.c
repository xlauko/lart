/* TAGS: sym c++ */
/* VERIFY_OPTS: --symbolic */
/* CC_OPTS: */

// V: v.O0 CC_OPT: -O0 TAGS: min
// V: v.O1 CC_OPT: -O1
// V: v.O2 CC_OPT: -O2
// V: v.Os CC_OPT: -Os
#include <lamp.h>
#include <stdint.h>

#include <cstdint>
#include <assert.h>

int main() {
    uint64_t array[ 4 ] = { 0 };
    uint64_t x = __lamp_any_i64();
    array[ 2 ] = x;
    assert( array[ 2 ] == x );
}
