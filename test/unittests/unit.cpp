#include <catch2/catch.hpp>

// #include <lamp.h>
extern "C" void* __lamp_wrap_i32( uint32_t v );
TEST_CASE( "wrap a unit value", "[unit]" ) {
    auto v = __lamp_wrap_i32(5);
    REQUIRE( v );
}