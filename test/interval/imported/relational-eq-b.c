// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= 2);
    assume(a <= 8);
    int32_t b = __lamp_any_i32();
    assume(a >= 8);
    assume(a <= 8);

    if (a != b) {
        if ( a != b ) {
            REACHABLE
        }
        else {
            UNREACHABLE
        }
    } else {
        if ( a == b ) {
            REACHABLE
        }
        else {
            UNREACHABLE
        }
    }
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}