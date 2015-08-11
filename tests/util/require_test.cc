#include "test_support/catch.hpp"
#include "util/require.h"

TEST_CASE("util::fail() works", "[util]")
{
    REQUIRE_THROWS(([&]() { au::util::fail("Nope"); })());
}

TEST_CASE("util::require() works", "[util]")
{
    REQUIRE_NOTHROW(([&]() { au::util::require(1 + 1 == 2); })());
    REQUIRE_THROWS(([&]() { au::util::require(1 + 1 != 2); })());
}
