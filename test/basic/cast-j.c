// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// REQUIRES: fix

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

void *foo() { /* creates inttoptr instuction */
    return __lamp_any_i64();
}


int main() {
    _Bool b = foo();
    assert( !b ); // CHECK: assertion !b failed
}
