// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= 1);
    assume(a <= 6);
    int i = 0;
    while (i < a) {
        ++i;
    }
    assume(i == a);
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}
