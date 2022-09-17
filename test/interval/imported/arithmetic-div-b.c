// TODO - signed
// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= 5);
    assume(a <= 20);

    int32_t b = __lamp_any_i32();
    assume(b >= -5);
    assume(b <= -1);

    int32_t c = a / b;
    if (c > 2) {
        if (a < 4) {
            UNREACHABLE
        }
        if (a > 8) {
            UNREACHABLE
        }
        if (b < 1) {
            UNREACHABLE
        }
        if (b > 2) {
            UNREACHABLE
        }
    } else {
        if (a != 4) {
            UNREACHABLE
        }
        if (b != 2) {
            UNREACHABLE
        }
    }
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}
