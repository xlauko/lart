/* TAGS: sym c++ */
/* VERIFY_OPTS: --symbolic */
/* CC_OPTS: */

// V: v.O0 CC_OPT: -O0 TAGS: min
// V: v.O1 CC_OPT: -O1
#include <lamp.h>
#include <stdint.h>
#include <assert.h>
#include <stdint.h>

int main() {
    int8_t arr[128] = { 0 };
    int32_t *i32ptr = ( int32_t * ) arr;
    int64_t *i64ptr = ( int64_t * ) arr;

    i64ptr[ 1 ] = __lamp_any_i64();
    int32_t expected = i32ptr[ 3 ];

    i32ptr[ 2 ] = __lamp_any_i32();

    assert( i32ptr[ 3 ] == expected );
    assert( i32ptr[ 2 ] != expected ); /* ERROR */
}
