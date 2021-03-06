// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>

#include "utils.h"

int main() {
    uint64_t x = __lamp_any_i64();
    uint64_t y = __lamp_any_i64();

    y = 0;

    ASSERT( y == 0 );
    ASSERT( x != y ); // CHECK: assertion `x != y' failed
}