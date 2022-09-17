// TODO - signed
// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= 500);
    assume(a <= 10000);

    int32_t b = __lamp_any_i32();
    assume(b >= -1000);
    assume(b <= 20000);

    int32_t c = a + b;
    if (c <= 5000) {
        int32_t t = a + b;
        if (t > 5000) {
            UNREACHABLE
        }
    } else {
        int32_t t = a + b;
        if (t <= 5000) {
            UNREACHABLE
        }
    }
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}