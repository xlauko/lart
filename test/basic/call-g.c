// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

#include "utils.h"

int id( int a ) { return a; }

int plus( int a, int b ) { return id( a ) + id( b ); }

int main() {
    int input = __lamp_any_i32();
    assert( plus( input, 10 ) == input + 10 );
    // CHECK-NOT: assertion {{.*}} failed
}
