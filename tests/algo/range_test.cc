// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "algo/range.h"
#include "algo/format.h"
#include "test_support/catch.h"

using namespace au;

static void do_test(algo::Range r, const std::vector<int> expected)
{
    std::vector<int> visited;
    for (const auto i : r)
        visited.push_back(i);
    REQUIRE(visited == expected);
}

TEST_CASE("Range", "[algo]")
{
    SECTION("Simple range iterating")
    {
        do_test(algo::range(0, 2), {0, 1});
    }
    SECTION("Range iterating with stride")
    {
        do_test(algo::range(0, 4, 2), {0, 2});
    }
    SECTION("Range iterating with unaligned stride")
    {
        do_test(algo::range(0, 5, 2), {0, 2, 4});
    }
    SECTION("Range iterating starting at negative offset")
    {
        do_test(algo::range(-5, 5), {-5, -4, -3, -2, -1, 0, 1, 2, 3, 4});
    }
    SECTION("Range iterating at negative offsets")
    {
        do_test(algo::range(-8, -4), {-8, -7, -6, -5});
    }
    SECTION("Reverse range iterating")
    {
        do_test(algo::range(5, -5, -1), {5, 4, 3, 2, 1, 0, -1, -2, -3, -4});
    }
}
