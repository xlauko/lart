// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>
#include "utils.h"

void fn(int i) {
    if (i)
        REACHABLE
}

int main() {
    int x = __lamp_any_i32();
    fn(x);
    // CHECK-COUNT-1: lart-reachable
}