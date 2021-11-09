// TODO:
#include <lamp.h>
#include <stdint.h>

#include <cstdint>
#include <assert.h>

int foo( int val ) {
    return val;
}

extern int ibari( int val );

int main() {
    int val = __lamp_any_i32();
    if ( val < 0 ) {
        auto fn = val < 0 ? &foo : &ibari;
        int ret = fn( val );
        assert( val < 0 );
    }
}