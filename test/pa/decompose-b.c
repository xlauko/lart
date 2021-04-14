// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s

#include <lamp-pointers.h>
#include <assert.h>
#include <stddef.h>
#include "utils.h"

int main()
{
    int val;
    int* ptr = (int*)__lamp_lift_objid( &val );
    *ptr = 10;
    uint32_t hi = (uintptr_t)(ptr) >> 32;
    uint32_t lo = (uintptr_t)(ptr) & 0x00000000FFFFFFFF;

    uint32_t* hip = (uint32_t*)__lamp_lift_objid( &hi );
    uint32_t* lop = (uint32_t*)__lamp_lift_objid( &lo );
    int* res = (int *)(((uintptr_t)(*hip) << 32) | (*lop));
    assert( res == ptr );
    assert( *res == 10 );
}