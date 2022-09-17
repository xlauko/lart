// TODO - bound = 0
// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= 1);
    assume(a <= 6);
    int i = __lamp_lower_i32(__lamp_twin(&a));
    if (i < 1 && i <= 6) {
        UNREACHABLE
    }
    REACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}
