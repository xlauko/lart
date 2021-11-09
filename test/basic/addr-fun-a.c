// TODO:
#include <lamp.h>
#include <stdint.h>
#include "utils.h"

#include <stdint.h>
#include <assert.h>

void foo( int val ) {
    assert( val < 0 );
}

void bar( int val ) {
    assert( val >= 0 );
}

int main() {
    int val = __lamp_any_i32();
    auto fn = val < 0 ? &foo : &bar;
    fn( val );
}
