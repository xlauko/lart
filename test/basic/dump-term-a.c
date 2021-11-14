// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>

int main() {
    int a = __lamp_any_i32();
    __lamp_dump(&a);
    // CHECK: var_1
}
