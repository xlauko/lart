// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= 6);
    assume(a <= 12);

    int32_t b = __lamp_any_i32();
    assume(b >= 2);
    assume(b <= 4);

    if (a < b) {
        UNREACHABLE
    } else {
        if (a < b) {
            UNREACHABLE
        }
    }
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}
