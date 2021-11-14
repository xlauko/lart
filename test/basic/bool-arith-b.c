// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include "utils.h"

#include <stdbool.h>
#include <assert.h>

int main()
{
    int a = __lamp_any_i32();
    int b = __lamp_any_i32();
    bool check = a == b;

    bool boolean = __lamp_any_i1();
    assert( check & boolean ); // CHECK: assertion check & boolean failed
}