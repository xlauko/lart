#include <catch.hpp>
#include <cc/annotations.hpp>

TEST_CASE( "annotations" )
{
    SECTION( "create" )
    {
        auto a = lart::annotation( "lart", "abstract" );
        REQUIRE( a.str() == "lart.abstract" );
    }
}
