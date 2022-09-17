// TODO - signed
// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include "utils.h"

#include <lamp.h>
#include <stdint.h>

int main() {
    int32_t a = __lamp_any_i32();
    assume(a >= -4);
    assume(a <= -1);

    int32_t b = __lamp_any_i32();
    assume(b >= -2);
    assume(b <= 4);

    int32_t c = a * b;
    if (c < 0) {
        int32_t t = a * b;
        if (t >= 0) {
            UNREACHABLE
        }

    } else {
        int32_t t = a * b;
        if (t < 0) {
            UNREACHABLE
        }
    }
}
