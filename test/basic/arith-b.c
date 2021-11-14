// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

#include "utils.h"

int main() {
    short a = __lamp_any_i16();
    int b = __lamp_any_i32();
    int r = a + b;
    __lamp_dump(&r);
    // CHECK: (bvadd ((_ sign_extend 16) var_1) var_2)
}
