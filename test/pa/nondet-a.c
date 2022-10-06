// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s
// REQUIRES: pa

#include <lamp-pointers.h>
#include <assert.h>
#include "utils.h"

int main()
{
    int *val = (int*)malloc(4);
    int *ptr = (int*)__lamp_any_ptr();
    ASSERT(ptr != val); // CHECK: assertion ptr != val failed
}
