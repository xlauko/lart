// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <assert.h>

#include "utils.h"

int plus( int a, int b ) { return a + b; }

int main() {
    int a = __lamp_any_i32();
    int b = plus( a, 10 );
    assert( b == a + 10 );

    int c = plus( 10,  a );
    assert( c == 10 + a );

    // CHECK-NOT: assertion {{.*}} failed
}
