// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// REQUIRES: fix

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

#include "utils.h"

int main() {
    int x = __lamp_any_i32();
    if ( x == 0 ) {
        short *a = (short*) &x;
        ++(*a);
        assert( *a == 1 );
    }
    REACHABLE // CHECK-COUNT-2: lart-reachable
}
