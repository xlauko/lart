// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

#include "utils.h"

int plus(int *a) {
    *a += 1;
    return *a;
}

int main() {
    int *a;
    int x = __lamp_any_i32();
    a = &x;
    int r = plus(a);
    assert( x == r );
    REACHABLE // CHECK-COUNT-1: lart-reachable
}
