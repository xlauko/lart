// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s
// REQUIRES: pa

#include <lamp-pointers.h>
#include <assert.h>
#include "utils.h"

int arr[] = {1, 2, 3};

int main() {
    int * ptr = (int*)__lamp_any_ptr();
    assert(ptr != arr); // CHECK: assertion ptr != val failed
}
