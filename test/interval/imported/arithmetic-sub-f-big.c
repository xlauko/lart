// TODO - bound = 0
// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= 10000);
    assume(a <= 40000);

    int32_t b = __lamp_any_i32();
    assume(b >= 5000);
    assume(b <= 9500);

    int32_t c = a - b;
    if (c > 20000) {
        int32_t t = a - b;
        if (t <= 20000) {
            UNREACHABLE
        }
    } else {
        int32_t t = a - b;
        if (t > 20000) {
            UNREACHABLE
        }
    }
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}