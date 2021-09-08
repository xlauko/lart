// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>
#include "utils.h"

#include <stdbool.h>
#include <assert.h>

bool __VERIFIER_nondet_bool();

int main()
{
    bool a = __lamp_any_i1();
    bool b = !a;

    assert( a == b ); // CHECK: assertion `a == b' failed
}