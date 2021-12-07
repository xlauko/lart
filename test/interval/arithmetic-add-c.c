// RUN: %testrun %lartcc interval -lz3 %s -o %t | %filecheck %s

#include <stdint.h>
#include <lamp.h>
#include "utils.h"


int main()
{
    int16_t a = __lamp_lift_any();
    int16_t b = __lamp_lift_any();

    int16_t c = a + b;

    if ( c <= 10 ) {
        int16_t t = a + b;
        if ( t > 10 ) {
            UNREACHABLE
        }
        REACHABLE
    }
    else
    {
        UNREACHABLE
    }
    
    // CHECK-NOT: lart-unreachable
    // CHECK: lart-reachable
}