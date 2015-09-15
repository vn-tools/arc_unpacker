#include "test_support/catch.hh"
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

TEST_CASE("Range iterating with stride works", "[util]")
{
    std::string out;
    for (auto i : range(0, 4, 2))
        out += format("%d", i);
    REQUIRE(out == "02");
}

TEST_CASE("Range iterating with unaligned stride works", "[util]")
{
    std::string out;
    for (auto i : range(0, 5, 2))
        out += format("%d", i);
    REQUIRE(out == "024");
}

TEST_CASE("Range iterating starting at negative offset works", "[util]")
{
    std::string out;
    for (auto i : range(-5, 5))
        out += format("%d", i);
    REQUIRE(out == "-5-4-3-2-101234");
}

TEST_CASE("Range iterating at negative offsets works", "[util]")
{
    std::string out;
    for (auto i : range(-8, -4))
        out += format("%d", i);
    REQUIRE(out == "-8-7-6-5");
}

TEST_CASE("Reverse range iterating works", "[util]")
{
    std::string out;
    for (auto i : range(5, -5, -1))
        out += format("%d", i);
    REQUIRE(out == "543210-1-2-3-4");
}
