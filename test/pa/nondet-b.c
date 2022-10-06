// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s

#include <lamp-pointers.h>
#include <assert.h>
#include "utils.h"

int arr[] = {1, 2, 3};

int main() {
    int * ptr = (int*)__lamp_any_ptr();
    ASSERT(ptr != arr); // CHECK: assertion ptr != val failed
}
