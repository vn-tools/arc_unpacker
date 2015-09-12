#include "io/bit_reader.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::io;

static bstr from_bits(std::initializer_list<u8> s)
{
    bstr x;
    for (auto i : s)
        x += static_cast<char>(i);
    return x;
}

TEST_CASE("Reading missing bits throws exceptions", "[util][bit_reader]")
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

TEST_CASE("Reading single bits works", "[util][bit_reader]")
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

TEST_CASE("Reading multiple bits works", "[util][bit_reader]")
{
    BitReader reader(from_bits({ 0b10001111 }));
    REQUIRE(reader.get(7) == 0b1000111);
    REQUIRE(reader.get(1));
}

TEST_CASE("Reading multiple bytes works", "[util][bit_reader]")
{
    SECTION("Smaller test")
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

    SECTION("Bigger test")
    {
        BitReader reader(from_bits({ 0b10101010, 0b11110000, 0b00110011 }));
        REQUIRE(reader.get(1) == 1);
        REQUIRE(reader.get(23) == 0b01010101111000000110011);
    }

    SECTION("Max bit reader capacity test")
    {
        BitReader reader(from_bits({
            0b11001100, 0b10101010, 0b11110000, 0b00110011 }));
        REQUIRE(reader.get(32) == 0b11001100101010101111000000110011);
    }
}

TEST_CASE("Checking for EOF works", "[util][bit_reader]")
{
    BitReader reader("\x00\x00"_b);
    reader.get(7);
    REQUIRE(!reader.eof());
    reader.get(7);
    REQUIRE(!reader.eof());
    reader.get(1);
    REQUIRE(!reader.eof());
    reader.get(1);
    REQUIRE(reader.eof());
}

TEST_CASE("Checking size works", "[util][bit_reader]")
{
    BitReader reader1("\x00\x00"_b);
    REQUIRE(reader1.size() == 16);
    BitReader reader2("\x00"_b);
    REQUIRE(reader2.size() == 8);
    BitReader reader3(""_b);
    REQUIRE(reader3.size() == 0);
}
