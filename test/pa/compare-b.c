// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s
// REQUIRES: pa

#include <lamp-pointers.h>
#include <assert.h>

#include "utils.h"

int main()
{
    struct S { int x, y; } s;
    int *a = ( int * )__lamp_lift_objid( &s.x );
    int *b = ( int * )__lamp_lift_objid( &s.y );
    assert( b > a );
    REACHABLE // CHECK: lart-reachable
}
