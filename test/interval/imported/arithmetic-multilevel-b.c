// TODO - bound = 0
// RUN: %testrun %lartcc interval %s -g -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= 5);
    assume(a <= 15);

    int32_t b = __lamp_any_i32();
    assume(b >= 4);
    assume(b <= 8);

    int32_t r = a + b;
    if (r > 15) {
        if (r < 20) {
            int32_t t = a + b;
            if (t <= 15 || t > 20) {
                UNREACHABLE
            }
        } else {
            int32_t t = a + b;
            if (t < 20) {
                UNREACHABLE
            }
        }
    } else {
        if (r < 20) {
            int32_t t = a + b;
            if (t > 15) {
                UNREACHABLE
            }
        } else {
            UNREACHABLE
        }
    }
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}
