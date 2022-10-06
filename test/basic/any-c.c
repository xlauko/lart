// RUN: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s
// RUN: %testrun %lartcc unit %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>

#include "utils.h"

int main()
{
    uint8_t  a = __lamp_any_i8();
    uint16_t b = __lamp_any_i16();
    uint32_t c = __lamp_any_i32();
    uint64_t d = __lamp_any_i64();

    float  e = __lamp_any_f32();
    double f = __lamp_any_f64();
    REACHABLE // CHECK: lart-reachable
}

