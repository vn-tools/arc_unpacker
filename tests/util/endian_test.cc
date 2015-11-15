#include "test_support/catch.hh"
#include "util/endian.h"

using namespace au;
using namespace au::util;

TEST_CASE("Converting endianness", "[util]")
{
    const auto x = "\x12\x34\x56\x78";
    const auto big_endian = *reinterpret_cast<const u32*>(x) == 0x12345678;
    if (big_endian)
    {
        REQUIRE(from_little_endian<u8>(0x12) == 0x12);
        REQUIRE(from_little_endian<u16>(0x1234) == 0x3412);
        REQUIRE(from_little_endian<u32>(0x12345678) == 0x78563412);
        REQUIRE(from_big_endian<u8>(0x12) == 0x12);
        REQUIRE(from_big_endian<u16>(0x1234) == 0x1234);
        REQUIRE(from_big_endian<u32>(0x12345678) == 0x12345678);
        REQUIRE(to_little_endian<u16>(0x1234) == 0x3412);
        REQUIRE(to_little_endian<u32>(0x12345678) == 0x78563412);
        REQUIRE(to_big_endian<u16>(0x1234) == 0x1234);
        REQUIRE(to_big_endian<u32>(0x12345678) == 0x12345678);
    }
    else
    {
        REQUIRE(from_big_endian<u8>(0x12) == 0x12);
        REQUIRE(from_big_endian<u16>(0x1234) == 0x3412);
        REQUIRE(from_big_endian<u32>(0x12345678) == 0x78563412);
        REQUIRE(from_big_endian<u32>(0x12) == 0x12000000);
        REQUIRE(from_little_endian<u8>(0x12) == 0x12);
        REQUIRE(from_little_endian<u16>(0x1234) == 0x1234);
        REQUIRE(from_little_endian<u32>(0x12345678) == 0x12345678);
        REQUIRE(to_big_endian<u16>(0x1234) == 0x3412);
        REQUIRE(to_big_endian<u32>(0x12345678) == 0x78563412);
        REQUIRE(to_little_endian<u16>(0x1234) == 0x1234);
        REQUIRE(to_little_endian<u32>(0x12345678) == 0x12345678);
    }
}
