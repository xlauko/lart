// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s
// REQUIRES: pa

#include <lamp-pointers.h>
#include <assert.h>
#include "utils.h"

noinline void test( int * a, int * b ) { assert( a == b ); }

int main()
{
    int * x = (int *)__lamp_lift_objid( malloc( sizeof(int) ) );
    test( x, x );
    free( x );
    REACHABLE // CHECK: lart-reachable
}
