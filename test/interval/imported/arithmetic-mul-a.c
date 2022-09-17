// TODO - bound = 0
// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= 3);
    assume(a <= 7);

    int32_t b = __lamp_any_i32();
    assume(b >= 1);
    assume(b <= 9);
    int32_t c = a * b;
    if (c < 30) {
        int32_t t = a * b;
        int32_t y = t + 2;
        if (t >= 30) {
            UNREACHABLE
        }
    } else {
        int32_t t = a * b;
        if (t < 30) {
            UNREACHABLE
        }
    }
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}
