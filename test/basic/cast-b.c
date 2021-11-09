// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

int main() {
    int x = __lamp_any_i32();
    if ( x < 0 )
        return 0;
    short y = x;
    ++y;
    assert( y != -32768 ); // CHECK: assertion y != -32768 failed
}
