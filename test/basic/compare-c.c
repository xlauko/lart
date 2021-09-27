// TODO: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>

#include "utils.h"

int foo() { return 0; }

int main() {
    uint64_t x = __lamp_any_i64();
    ASSERT( x != foo() ); // CHECK: assertion `x != foo()' failed
}
