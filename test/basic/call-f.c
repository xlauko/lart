// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

#include "utils.h"

int plus( int a, int b ) { return a + b; }

int main() {
    int a = __lamp_any_i32();
    int b = __lamp_any_i32();
    assert( plus( a, 10 ) == a + 10 );
    assert( plus( 10, b ) == 10 + b );
    assert( plus( a, b ) == a + b );
    REACHABLE // CHECK: lart-reachable
}
