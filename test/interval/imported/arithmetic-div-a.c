// TODO - all
// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= 4);
    assume(a <= 8);

    int32_t b = __lamp_any_i32();
    assume(b >= 1);
    assume(b <= 2);

    int32_t c = a / b;
    if (c > 2) {
        int32_t t = a / b;
        if (t <= 2) {
            UNREACHABLE
        }
    } else {
        int32_t t = a / b;
        if (t > 2) {
            UNREACHABLE
        }
    }
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}
