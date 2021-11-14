// RUN: %testrun %lartcc term -lz3 %s -o %t 2>&1 | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

int main() {
    int x = __lamp_any_i32();
    if ( x < 0 )
        return 0;
    short y = x;
    ++y;
    assert( y != SHRT_MIN ); // CHECK: assertion y != SHRT_MIN failed
}
