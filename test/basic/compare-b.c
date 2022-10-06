// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

#include "utils.h"

int main() {
    uint64_t x = __lamp_any_i64();
    uint64_t y = __lamp_any_i64();

    y = 0;

    assert( y == 0 );
    assert( x != y ); // CHECK: assertion x != y failed
}
