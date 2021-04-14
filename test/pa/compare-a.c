// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s

#include <lamp-pointers.h>
#include <assert.h>
#include "utils.h"

int main()
{
    int arr[3];
    int *p = (int*)__lamp_lift_objid( arr );
    int *a = p;
    int *b = p + 1;
    assert( a != b ); 
    REACHABLE // CHECK: lart-reachable
}
