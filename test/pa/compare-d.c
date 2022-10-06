// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s
// REQUIRES: pa

#include <lamp-pointers.h>
#include <assert.h>
#include "utils.h"

noinline void test( char * a, char * b ) { assert( a != b ); }

int main()
{
    char * x = (char *)__lamp_lift_objid( malloc(1) );
    char * y = (char *)__lamp_lift_objid( malloc(1) );
    if ( x && y )
        test( x, y );
    free( x );
    free( y );
    REACHABLE // CHECK: lart-reachable
}
