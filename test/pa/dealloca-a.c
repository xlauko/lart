// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s

#include <lamp-pointers.h>
#include <assert.h>
#include "utils.h"

int main()
{
    void *p = NULL;
    void *x = __lamp_lift_objid( p );

    struct S { int x, y; } s;
    void *y = __lamp_lift_objid( &s.y );
    REACHABLE // CHECK: lart-reachable
}