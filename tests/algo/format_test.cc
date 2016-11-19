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
