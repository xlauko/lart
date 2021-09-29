// TODO: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <assert.h>

#include "utils.h"

// test output argument
void init( int *i, int v ) { *i = v; }

int main() {
    int val = __lamp_any_i32();
    int i;

    init( &i, val );

    assert( i == val );
    
    // CHECK-NOT: assertion {{.*}} failed
}
