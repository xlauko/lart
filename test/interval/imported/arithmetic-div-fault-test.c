// TODO - signed, zero division
// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= -4);
    assume(a <= -1);

    int32_t b = __lamp_any_i32();
    assume(b >= 0);
    assume(b <= 4);
    int32_t t = a / b;

    int32_t c = __lamp_any_i32();
    assume(c >= -4);
    assume(c <= 4);
    assume()
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}
