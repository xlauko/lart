// TODO - bound = 0
// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= 2);
    assume(a <= 10);

    int32_t b = __lamp_any_i32();
    assume(b >= 4);
    assume(b <= 10);

    int32_t c = __lamp_any_i32();
    assume(c >= 3);
    assume(c <= 5);

    int32_t d = __lamp_any_i32();
    assume(d >= 5);
    assume(d <= 10);

    if (a + b >= d - c) {
        int32_t r1 = a + b;
        if (r1 < d - c) {
            UNREACHABLE
        }
    } else {
        int32_t r1 = a + b;
        if (r1 >= d - c) {
            UNREACHABLE
        }
    }
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}
