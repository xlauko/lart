// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <stdint.h>
#include <assert.h>

#include <lamp.h>
#include "utils.h"

uint32_t __lamp_any_i32(void); // test bitcast from different function type

int main() {
    int x = __lamp_any_i32();
    assert( x <= 0 ); // CHECK: assertion x <= 0 failed
}
