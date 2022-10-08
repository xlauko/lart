// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// REQUIRES: fix

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

#include "utils.h"

void plus(int *a) {
    *a += 1;
}

int main() {
    int x = __lamp_any_i32();
    if ( x == 0 ) {
        plus(&x);
        assert( x == 1 );
    }
    REACHABLE // CHECK-COUNT-2: lart-reachable
}
