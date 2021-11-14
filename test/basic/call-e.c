// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

bool is_zero( int v ) { return v == 0; }

int main() {
    int input = __lamp_any_i32();
    assert( is_zero( input ) ); // CHECK: assertion is_zero( input ) failed
}
