// RUN: %testrun %lartcc++ pa -lz3 %s -o %t | %filecheck %s
// REQUIRES: pa

#include <lamp-pointers.h>
#include <assert.h>
#include "utils.h"

int main()
{
    char * x = (char *)__lamp_lift_objid( malloc(1) );
    if ( reinterpret_cast< uintptr_t >( x ) & 0x1 )
        free( x );
    REACHABLE // CHECK: lart-reachable
}
