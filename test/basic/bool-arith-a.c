// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>
#include "utils.h"

#include <stdbool.h>
#include <assert.h>

int main()
{
    bool a = __lamp_any_i1();
    bool b = __lamp_any_i1();
    assert( a && b ); // CHECK: assertion a && b failed
}