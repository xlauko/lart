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
    assert( x + 1 == y ); // CHECK: assertion x + 1 == y failed
}
