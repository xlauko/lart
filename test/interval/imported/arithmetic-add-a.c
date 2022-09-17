// RUN: %testrun %lartcc interval %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= 11);
    assume(a <= 20);

    int32_t b = __lamp_any_i32();
    assume(b >= 7);
    assume(b <= 9);

    int32_t c = a + b;
    if (c <= 25) {
        int32_t t = a + b;
        if (t > 25) {
            UNREACHABLE
        }
    } else {
        int32_t t = a + b;
        if (t <= 25) {
            UNREACHABLE
        }
    }
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}