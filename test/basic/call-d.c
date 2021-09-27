// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>
#include "utils.h"

int fn(int i, int j) {
    if (j)
        return j;
    return i;
}

int main() {
    int x = 0;
    int y = __lamp_any_i32();
    if (fn(x, y))
        REACHABLE
    if (fn(y, x))
        UNREACHABLE
    // CHECK-NOT: lart-unreachable
    // CHECK-COUNT-1: lart-reachable
}