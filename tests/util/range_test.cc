#include "test_support/catch.hh"
#include "util/range.h"
#include "util/format.h"

using namespace au::util;

static void do_test(au::util::RangeImpl r, const std::vector<int> expected)
{
    std::vector<int> visited;
    for (const auto i : r)
        visited.push_back(i);
    REQUIRE(visited == expected);
}

TEST_CASE("util::range", "[util]")
{
    SECTION("Simple range iterating")
    {
        do_test(range(0, 2), {0, 1});
    }
    SECTION("Range iterating with stride")
    {
        do_test(range(0, 4, 2), {0, 2});
    }
    SECTION("Range iterating with unaligned stride")
    {
        do_test(range(0, 5, 2), {0, 2, 4});
    }
    SECTION("Range iterating starting at negative offset")
    {
        do_test(range(-5, 5), {-5, -4, -3, -2, -1, 0, 1, 2, 3, 4});
    }
    SECTION("Range iterating at negative offsets")
    {
        do_test(range(-8, -4), {-8, -7, -6, -5});
    }
    SECTION("Reverse range iterating", "[util]")
    {
        do_test(range(5, -5, -1), {5, 4, 3, 2, 1, 0, -1, -2, -3, -4});
    }
}
