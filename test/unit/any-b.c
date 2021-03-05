// RUN: %lartcc unit %s -o %t
// RUN: export DFSAN_OPTIONS=warn_unimplemented=0 && %t 2>&1 | %filecheck %s

#include <lamp.h>
#include "utils.h"

int main()
{
    uint8_t  a = __lamp_any_i8();
    uint16_t b = __lamp_any_i16();
    uint32_t c = __lamp_any_i32();
    uint64_t d = __lamp_any_i64();

    float  e = __lamp_any_f32();
    double f = __lamp_any_f64();
}

