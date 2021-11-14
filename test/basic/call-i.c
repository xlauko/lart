// TODO: %testrun %lartcc term -lz3 %s -o %t | %filecheck %s

#include <lamp.h>
#include <stdint.h>
#include <assert.h>

#include "utils.h"

int* init_and_pass( int* val ) {
    *val = __lamp_any_i32();
    return val;
}

int process( int* addr ) {
    auto ret = init_and_pass( addr );
    return *ret;
}

int main() {
    int i;
    int j = process( &i );
    assert( i == j );
    
    // CHECK-NOT: assertion {{.*}} failed
}



