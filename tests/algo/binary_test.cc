#include "algo/binary.h"
#include "test_support/catch.h"

using namespace au;

TEST_CASE("Binary utilities", "[algo]")
{
    SECTION("Xor with u8 key")
    {
        REQUIRE(algo::xor("test"_b, 1) == "udru"_b);
    }

    SECTION("Xor with bstr key")
    {
        REQUIRE(algo::xor("test"_b, "\x01\x02"_b) == "ugrv"_b);
    }

    SECTION("Xor with empty bstr key")
    {
        REQUIRE_THROWS(algo::xor("test"_b, ""_b));
    }

    SECTION("Bit rotation")
    {
        REQUIRE(algo::rotl<u16>(1, 0) == 0b00000000'00000001);
        REQUIRE(algo::rotl<u16>(1, 1) == 0b00000000'00000010);
        REQUIRE(algo::rotl<u16>(1, 8) == 0b00000001'00000000);
        REQUIRE(algo::rotl<u8>(1, 7) == 0b10000000);
        REQUIRE(algo::rotl<u8>(1, 8) == 0b00000001);
        REQUIRE(algo::rotl<u8>(1, 9) == 0b00000010);

        REQUIRE(algo::rotr<u16>(1, 0) == 0b00000000'00000001);
        REQUIRE(algo::rotr<u16>(1, 1) == 0b10000000'00000000);
        REQUIRE(algo::rotr<u16>(1, 8) == 0b00000001'00000000);
        REQUIRE(algo::rotr<u8>(1, 7) == 0b00000010);
        REQUIRE(algo::rotr<u8>(1, 8) == 0b00000001);
        REQUIRE(algo::rotr<u8>(1, 9) == 0b10000000);
    }
}
