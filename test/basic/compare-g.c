// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>

#include "utils.h"

int main() {
    int input = __lamp_any_i32();
    if ((input != 1) && (input != 2) && (input != 3) && (input != 4) && (input != 5) && (input != 6)) 
        return -2;
    REACHABLE
    // CHECK-COUNT-5: lart-reachable
}
