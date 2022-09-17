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
    assume(c <= 6);

    int32_t r = a * (b + c);
    if (r >= 60) {
        if (r < 120) {
            int32_t t = a * (b + c);
            if (t < 60 || t >= 120) {
                UNREACHABLE
            }
        } else {
            int32_t t = a * (b + c);
            if (t < 120) {
                UNREACHABLE
            }
        }
    } else {
        if (r < 120) {
            int32_t t = a * (b + c);
            if (t >= 60) {
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
