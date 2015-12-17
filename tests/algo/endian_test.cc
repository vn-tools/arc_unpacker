#include "algo/endian.h"
#include "test_support/catch.h"

using namespace au;

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
