// TODO: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

int main() {
    union {
        int x;
        short y;
    } u;

    u.y = (int)__lamp_any_i32();

    if ( (short) u.x < 0 )
        return 0;
    ++u.y;
    assert( u.y == -32768 || u.y >= 1 );
    assert( u.y != -32768 ); // CHECK: assertion u.y != -32768 failed
}
