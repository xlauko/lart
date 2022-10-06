// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include "utils.h"

bool is_zero( int v ) { return v == 0; }

int main() {
    int input = __lamp_any_i32();
    ASSERT( is_zero( input ) ); // CHECK: assertion is_zero( input ) failed
}
