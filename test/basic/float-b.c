/* TAGS: error sym c++ float todo */
/* VERIFY_OPTS: --symbolic */
/* CC_OPTS: */

// V: v.O0 CC_OPT: -O0 TAGS: error min
// V: v.O1 CC_OPT: -O1
// V: v.O2 CC_OPT: -O2
// V: v.Os CC_OPT: -Os
#include <lamp.h>
#include <stdint.h>

#include <cstdint>
#include <assert.h>

int main() {
    float x = __lamp_any_float32();
    float y = __lamp_any_float32();

    float z = x + y; // test addition
    assert( z == y ); /* ERROR */
}
