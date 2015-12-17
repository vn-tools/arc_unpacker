#include "algo/format.h"
#include "test_support/catch.h"

using namespace au;

TEST_CASE("Formatting strings", "[algo]")
{
    SECTION("Small strings")
    {
        REQUIRE(algo::format("%d", 1) == "1");
        REQUIRE(algo::format("%02d", 5) == "05");
        REQUIRE(algo::format("%.02f", 3.14f) == "3.14");
    }

    SECTION("Big strings")
    {
        const std::string big(1000, '-');
        REQUIRE(big.size() == 1000);
        REQUIRE(algo::format(big + "%d", 1) == big + "1");
        REQUIRE(algo::format(big + "%02d", 5) == big + "05");
        REQUIRE(algo::format(big + "%.02f", 3.14f) == big + "3.14");
    }
}
