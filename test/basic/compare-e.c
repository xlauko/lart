// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <stdint.h>

#include "utils.h"

uint32_t __lamp_any_i32(void); // test bitcast from different function type

int main() {
    int x = __lamp_any_i32();
    ASSERT( x <= 0 ); // CHECK: assertion `x <= 0' failed
}
