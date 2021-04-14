// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s

#include <lamp-pointers.h>
#include <assert.h>
#include <stddef.h>
#include "utils.h"

int main()
{
    int val = 42;
    auto ptr = (int*)__lamp_lift_objid( &val );
    uint32_t hi = (uintptr_t)(ptr) >> 32;
    uint32_t lo = (uintptr_t)(ptr) & 0x00000000FFFFFFFF;
    int *res = (int *)(((uintptr_t)hi << 32) | lo);
    assert( *res == val );
    REACHABLE // CHECK: lart-reachable
}