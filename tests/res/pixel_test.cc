#include <limits>
#include "res/pixel.h"
#include "test_support/catch.hh"

using namespace au;

TEST_CASE("Pixel properties", "[core][types]")
{
    REQUIRE(sizeof(res::Pixel) == 4);
    REQUIRE(std::is_pod<res::Pixel>::value);
}
