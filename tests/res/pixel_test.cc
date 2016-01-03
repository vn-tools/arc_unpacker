#include "res/pixel.h"
#include <limits>
#include "test_support/catch.h"

using namespace au;

TEST_CASE("Pixel properties", "[res]")
{
    REQUIRE(sizeof(res::Pixel) == 4);
    REQUIRE(std::is_pod<res::Pixel>::value);
}
