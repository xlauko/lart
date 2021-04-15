// RUN: %testrun %lartcc++ pa -lz3 %s -o %t | %filecheck %s

#include <lamp-pointers.h>
#include <assert.h>
#include <stdlib.h>
#include "utils.h"


void pointer_xor_swap(char **x, char **y)
{
    uintptr_t a = (uintptr_t)*x;
    uintptr_t b = (uintptr_t)*y;

    a = a ^ b;
    b = a ^ b;
    a = a ^ b;

    *x = (char*)a;
    *y = (char*)b;
}

int main () {
    char * x = (char *)__lamp_lift_objid( malloc(1) );
    char * y = (char *)__lamp_lift_objid( malloc(1) );

    char * u = x;
    char * v = y;

    pointer_xor_swap( &u, &v );
    assert( x == v );
    assert( y == u );

    free( x );
    free( y );
}