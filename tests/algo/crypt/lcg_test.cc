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

#include "algo/crypt/lcg.h"
#include "test_support/catch.h"

using namespace au::algo::crypt;

TEST_CASE("Linear congruential generators", "[algo][crypt]")
{
    SECTION("Microsoft Visual C++")
    {
        Lcg l(LcgKind::MicrosoftVisualC, 0xDEADBEEF);
        REQUIRE(l.next() == 18337);
        REQUIRE(l.next() == 16920);
        REQUIRE(l.next() == 18732);
        REQUIRE(l.next() == 19770);
        REQUIRE(l.next() == 16163);
        REQUIRE(l.next() == 20831);
        REQUIRE(l.next() == 31795);
        REQUIRE(l.next() == 19149);
    }

    SECTION("Park&Miller")
    {
        Lcg l(LcgKind::ParkMiller, 0xDEADBEEF);
        REQUIRE(l.next() == 193);
        REQUIRE(l.next() == 79);
        REQUIRE(l.next() == 133);
        REQUIRE(l.next() == 113);
        REQUIRE(l.next() == 140);
        REQUIRE(l.next() == 117);
        REQUIRE(l.next() == 67);
        REQUIRE(l.next() == 114);
    }

    SECTION("Park&Miller revised")
    {
        Lcg l(LcgKind::ParkMillerRevised, 0xDEADBEEF);
        REQUIRE(l.next() == 246);
        REQUIRE(l.next() == 50);
        REQUIRE(l.next() == 60);
        REQUIRE(l.next() == 141);
        REQUIRE(l.next() == 213);
        REQUIRE(l.next() == 234);
        REQUIRE(l.next() == 179);
        REQUIRE(l.next() == 38);
    }
}
