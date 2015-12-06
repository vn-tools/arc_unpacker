#include "util/format.h"
#include "test_support/catch.hh"

using namespace au;

TEST_CASE("Formatting strings", "[util]")
{
    SECTION("Small strings")
    {
        REQUIRE(util::format("%d", 1) == "1");
        REQUIRE(util::format("%02d", 5) == "05");
        REQUIRE(util::format("%.02f", 3.14f) == "3.14");
    }

    SECTION("Big strings")
    {
        const std::string big(1000, '-');
        REQUIRE(big.size() == 1000);
        REQUIRE(util::format(big + "%d", 1) == big + "1");
        REQUIRE(util::format(big + "%02d", 5) == big + "05");
        REQUIRE(util::format(big + "%.02f", 3.14f) == big + "3.14");
    }
}
