// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <stdint.h>
#include <assert.h>

#include "utils.h"

int main() {
    uint64_t array[ 4 ] = { 1, 2, 3, 4 };
    array[ 0 ] = __lamp_any_i64();

    if ( array[ 0 ] > array[ 3 ] ) {
        assert( array[ 0 ] - array[ 3 ] > 0 );
    }
    REACHABLE // CHECK-COUNT-2: lart-reachable
}
