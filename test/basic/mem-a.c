// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

#include "utils.h"

int main() {
    int x = __lamp_any_i32();
    int *a = &x;
    int y = *a;
    assert( x == y );
    REACHABLE // CHECK: lart-reachable
}
