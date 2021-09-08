// RUN: %testrun %lartcc trivial %s -o %t | %filecheck %s
// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>
#include "utils.h"

int main() {
    uint8_t x = __lamp_any_i8();
    uint8_t y = __lamp_lift_i8( 10 );
    uint8_t z = x + y;
    ASSERT( x != z ); // CHECK: assertion `x != z' failed
}
