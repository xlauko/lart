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
    int x = 0;
    int y = __lamp_any_i32();
    fn(x, y);
    fn(y, x);
    // CHECK-COUNT-2: lart-reachable
}