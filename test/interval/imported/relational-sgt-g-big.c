// TODO - signed
// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= -20000);
    assume(a <= 10000);

    int32_t b = __lamp_any_i32();
    assume(b >= -10000);
    assume(b <= 20000);

    if (a > b) {
        if (a <= b) {
            UNREACHABLE
        }
    } else {
        if (a > b) {
            UNREACHABLE
        }
    }
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}