#include "test_support/catch.hh"
#include "util/format.h"

using namespace au::util;

TEST_CASE("Formatting strings", "[util]")
{
    SECTION("Small strings")
    {
        REQUIRE(format("%d", 1) == "1");
        REQUIRE(format("%02d", 5) == "05");
        REQUIRE(format("%.02f", 3.14f) == "3.14");
    }

    SECTION("Big strings")
    {
        std::string big_string(1000, '-');
        REQUIRE(big_string.size() == 1000);
        REQUIRE(format(big_string + "%d", 1) == big_string + "1");
        REQUIRE(format(big_string + "%02d", 5) == big_string + "05");
        REQUIRE(format(big_string + "%.02f", 3.14f) == big_string + "3.14");
    }
}
