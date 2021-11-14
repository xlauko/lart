// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include "utils.h"

void fn(int i, int j) {
    if (i)
        REACHABLE
    if (j)
        REACHABLE
}

int main() {
    int x = __lamp_any_i32();
    int y = __lamp_any_i32();
    fn(x, y);
    // CHECK-COUNT-2: lart-reachable
}