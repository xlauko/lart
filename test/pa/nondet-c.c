// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s
// REQUIRES: pa

#include <lamp-pointers.h>
#include <assert.h>
#include "utils.h"

int main()
{
    int *val = (int*)malloc(4);
    int *ptr = (int*)__lamp_any_ptr();
    if (val == ptr) {
        assert(ptr == val);
        REACHABLE // CHECK: lart-reachable
    }
    REACHABLE // CHECK: lart-reachable
}
