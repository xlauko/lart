// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>

#include "utils.h"

int main() {
    int i = __lamp_any_i32();
    assert( i != 0 ); // CHECK: assertion i != 0 failed
}
