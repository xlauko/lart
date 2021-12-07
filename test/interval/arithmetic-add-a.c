// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include <stdint.h>
#include <lamp.h>
#include "utils.h"

int main() {
    uint8_t a = __lamp_any_i8();
    uint8_t b = __lamp_any_i8();

    uint8_t c = a + b;
    if ( c <= 25 ) {
        uint8_t t = a + b;
        if ( t > 25 ) {
            UNREACHABLE
        }
        REACHABLE
    }
    else {
        uint8_t t = a + b;
        if ( t <= 25 ) {
            UNREACHABLE
        }
        REACHABLE
    }

    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}