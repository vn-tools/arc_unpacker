#include "algo/binary.h"
#include "test_support/catch.hh"

using namespace au;

TEST_CASE("Binary utilities", "[algo]")
{
    SECTION("Bit rotation")
    {
        REQUIRE(algo::rotl<u16>(1, 0) == 0b00000000'00000001);
        REQUIRE(algo::rotl<u16>(1, 1) == 0b00000000'00000010);
        REQUIRE(algo::rotl<u16>(1, 8) == 0b00000001'00000000);
        REQUIRE(algo::rotl<u8>(1, 8) == 0b00000001);
        REQUIRE(algo::rotl<u8>(1, 9) == 0b00000010);

        REQUIRE(algo::rotr<u16>(1, 0) == 0b00000000'00000001);
        REQUIRE(algo::rotr<u16>(1, 1) == 0b10000000'00000000);
        REQUIRE(algo::rotr<u16>(1, 8) == 0b00000001'00000000);
        REQUIRE(algo::rotr<u8>(1, 8) == 0b00000001);
        REQUIRE(algo::rotr<u8>(1, 9) == 0b10000000);
    }
}
