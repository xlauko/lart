// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <assert.h>


int nondet() {
    return (int)__lamp_any_i32();
}

int main() {
    int x = nondet();
    if ( (short) x < 0 )
        return 0;
    short y = *(short *)( &x );
    ++y;
    assert( y == -32768 || y >= 1 );
    assert( y != -32768 ); // CHECK: assertion y != -32768 failed
}
