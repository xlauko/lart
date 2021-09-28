// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>

int main() {
    int a = __lamp_any_i32();
    __lamp_dump(&a);
    // CHECK: unit
}
