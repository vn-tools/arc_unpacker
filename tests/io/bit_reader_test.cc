#include "io/bit_reader.h"
#include "io/buffered_io.h"
#include "test_support/catch.hpp"

using namespace au::io;

TEST_CASE("Reading missing bits throws exceptions")
{
    BitReader reader("", 0);
    try
    {
        reader.get(1);
        REQUIRE(0);
    }
    catch (...)
    {
    }
}

TEST_CASE("Reading single bits works")
{
    BitReader reader("\x8F", 2); //10001111
    REQUIRE(reader.get(1));
    REQUIRE(!reader.get(1));
    REQUIRE(!reader.get(1));
    REQUIRE(!reader.get(1));
    REQUIRE(reader.get(1));
    REQUIRE(reader.get(1));
    REQUIRE(reader.get(1));
    REQUIRE(reader.get(1));
}

TEST_CASE("Reading multiple bits works")
{
    BitReader reader("\x8F", 2); //10001111
    REQUIRE(reader.get(7) == (0x8F >> 1));
    REQUIRE(reader.get(1));
}

TEST_CASE("Reading multiple bytes works")
{
    BitReader reader("\x8F\x8F", 2); //10001111
    REQUIRE(reader.get(7) == (0x8F >> 1));
    REQUIRE(reader.get(1));

    REQUIRE(reader.get(1));
    REQUIRE(!reader.get(1));
    REQUIRE(reader.get(4) == 3);
    REQUIRE(reader.get(2) == 3);
    try
    {
        reader.get(1);
        REQUIRE(0);
    }
    catch (...)
    {
    }
}
