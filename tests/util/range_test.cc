#include "test_support/catch.hpp"
#include "util/range.h"
#include "util/format.h"

using namespace au::util;

TEST_CASE("Simple range iterating works", "[util]")
{
    std::string out;
    for (auto i : range(0, 2))
        out += format("%d", i);
    REQUIRE(out == "01");
}

TEST_CASE("Range iterating starting at negative offset works")
{
    std::string out;
    for (auto i : range(-5, 5))
        out += format("%d", i);
    REQUIRE(out == "-5-4-3-2-101234");
}

TEST_CASE("Reverse range iterating works")
{
    std::string out;
    for (auto i : range(5, 0))
        out += format("%d", i);
    REQUIRE(out == "54321");
}

TEST_CASE("Reverse range iterating at negative offsets works")
{
    std::string out;
    for (auto i : range(-4, -8))
        out += format("%d", i);
    REQUIRE(out == "-4-5-6-7");
}
