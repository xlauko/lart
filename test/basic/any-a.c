// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>
#include "utils.h"

int main() {
    uint8_t x = __lamp_any_i8();
    if ( x ) {
        REACHABLE
    } else {
        REACHABLE
    }
    // CHECK-COUNT-2: lart-reachable
}
