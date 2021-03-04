// RUN: %lartcc unit %s -o %t
// RUN: export DFSAN_OPTIONS=warn_unimplemented=0 && %t 2>&1 | %filecheck %s

#include <lamp.h>
#include "utils.hpp"

int main() {
    uint8_t x = __lamp_any_i8();
    if ( x )
        lart::reachable();
    else
        lart::reachable();
    // CHECK-COUNT-2: lart::reachable
}

