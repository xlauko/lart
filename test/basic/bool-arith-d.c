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
    __lamp_dump(&y);
    // CHECK: ((_ extract 15 0) var_1)
    y++;
    __lamp_dump(&y);
    // CHECK: (bvadd ((_ extract 15 0) var_1) #x0001)
}
