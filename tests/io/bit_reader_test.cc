#include "io/bit_reader.h"
#include "io/buffered_io.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::io;

TEST_CASE("Reading missing bits throws exceptions")
{
    BitReader reader(""_b);
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
    BitReader reader("\x8F"_b); //10001111
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
    BitReader reader("\x8F"_b); //10001111
    REQUIRE(reader.get(7) == (0x8F >> 1));
    REQUIRE(reader.get(1));
}

TEST_CASE("Reading multiple bytes works")
{
    BitReader reader("\x8F\x8F"_b); //10001111
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
