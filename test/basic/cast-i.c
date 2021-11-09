// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

int nondet() {
    return __lamp_any_i32();
}

int main() {
    int x;
    x = nondet();
    if ( x < 0 )
        return 0;
    short *y = ( short * )( &x );
    *y = 0;
    ++(*y);
    assert( *y == 1 );
    assert( x <= 0x7fff0001 ); // CHECK: assertion x <= 0x7fff0001 failed
}
