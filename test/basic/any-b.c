// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include "utils.h"

int main() {
    uint8_t x = __lamp_any_i8();
    uint8_t y = __lamp_any_i8();
    if ( x < y ) {
        if ( x > y ) {
            UNREACHABLE
        }
        REACHABLE
    }
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}
