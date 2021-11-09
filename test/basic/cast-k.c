// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

#include "utils.h"

int main() {
    int x = __lamp_any_i32();
    if ( x >= SHRT_MIN && x <= SHRT_MAX ) {
        short y = (short)x;
        assert( x == y );
    }

    REACHABLE // CHECK-COUNT-2: lart-reachable
}
