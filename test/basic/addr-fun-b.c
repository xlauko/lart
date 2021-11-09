// TODO:

#include <lamp.h>
#include <stdint.h>

#include <cstdint>
#include <assert.h>

void foo( int val ) {
    assert( val < 0 );
}

extern void vbari( int val );

int main() {
    int val = __lamp_any_i32();
    auto fn = val < 0 ? &foo : &vbari;
    fn( val ); /* ERROR */
}