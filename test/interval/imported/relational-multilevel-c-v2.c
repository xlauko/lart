// TODO - all bounds
// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= 1);
    assume(a <= 25);

    int32_t b = __lamp_any_i32();
    assume(b >= 13);
    assume(b <= 18);

    int32_t c = __lamp_any_i32();
    assume(c >= 11);
    assume(c <= 17);

    if (a < c && a == b) {
        if (a >= c) {
            UNREACHABLE
        }
        if (b >= c) {
            UNREACHABLE
        }
    } else {
        if (a < c || a != b) {
            UNREACHABLE
        }
    }
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}