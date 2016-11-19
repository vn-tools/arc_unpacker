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

#include "algo/endian.h"
#include "test_support/catch.h"

using namespace au;

TEST_CASE("Machine endianness", "[algo]")
{
    const char *temp = "\x44\x33\x22\x11";
    const auto temp2 = *reinterpret_cast<const u32*>(temp);
    INFO("arc_unpacker is designed for little-endian machines");
    REQUIRE(temp2 == 0x11223344);
}

TEST_CASE("Converting endianness", "[algo]")
{
    const auto x = "\x12\x34\x56\x78";
    const auto big_endian = *reinterpret_cast<const u32*>(x) == 0x12345678;
    if (big_endian)
    {
        REQUIRE(algo::from_little_endian<u8>(0x12) == 0x12);
        REQUIRE(algo::from_little_endian<u16>(0x1234) == 0x3412);
        REQUIRE(algo::from_little_endian<u32>(0x12345678) == 0x78563412);
        REQUIRE(algo::from_big_endian<u8>(0x12) == 0x12);
        REQUIRE(algo::from_big_endian<u16>(0x1234) == 0x1234);
        REQUIRE(algo::from_big_endian<u32>(0x12345678) == 0x12345678);
        REQUIRE(algo::to_little_endian<u16>(0x1234) == 0x3412);
        REQUIRE(algo::to_little_endian<u32>(0x12345678) == 0x78563412);
        REQUIRE(algo::to_big_endian<u16>(0x1234) == 0x1234);
        REQUIRE(algo::to_big_endian<u32>(0x12345678) == 0x12345678);
    }
    else
    {
        REQUIRE(algo::from_big_endian<u8>(0x12) == 0x12);
        REQUIRE(algo::from_big_endian<u16>(0x1234) == 0x3412);
        REQUIRE(algo::from_big_endian<u32>(0x12345678) == 0x78563412);
        REQUIRE(algo::from_big_endian<u32>(0x12) == 0x12000000);
        REQUIRE(algo::from_little_endian<u8>(0x12) == 0x12);
        REQUIRE(algo::from_little_endian<u16>(0x1234) == 0x1234);
        REQUIRE(algo::from_little_endian<u32>(0x12345678) == 0x12345678);
        REQUIRE(algo::to_big_endian<u16>(0x1234) == 0x3412);
        REQUIRE(algo::to_big_endian<u32>(0x12345678) == 0x78563412);
        REQUIRE(algo::to_little_endian<u16>(0x1234) == 0x1234);
        REQUIRE(algo::to_little_endian<u32>(0x12345678) == 0x12345678);
    }
}
