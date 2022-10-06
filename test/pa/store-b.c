// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s
// REQUIRES: pa

#include <lamp-pointers.h>
#include <assert.h>
#include "utils.h"

int main()
{
    int a = 1;
    int *p = (int*)__lamp_lift_objid( &a );
    *p = 4;
    assert( a == 4 );
    REACHABLE // CHECK: lart-reachable
}
