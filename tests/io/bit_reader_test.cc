#include "io/bit_reader.h"
#include "util/range.h"
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

TEST_CASE("Reading missing bits throws exceptions", "[io]")
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

TEST_CASE("Reading single bits works", "[io]")
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

TEST_CASE("Reading multiple bits works", "[io]")
{
    BitReader reader(from_bits({ 0b10001111 }));
    REQUIRE(reader.get(7) == 0b1000111);
    REQUIRE(reader.get(1));
}

TEST_CASE("Reading multiple bytes works", "[io]")
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

    SECTION("Max bit reader capacity test (unaligned)")
    {
        BitReader reader(from_bits({
            0b11001100, 0b10101010, 0b11110000, 0b00110011 }));
        REQUIRE(reader.get(32) == 0b11001100'10101010'11110000'00110011);
    }

    SECTION("Max bit reader capacity test (aligned)")
    {
        BitReader reader(from_bits({
            0b11001100, 0b10101010, 0b11110000, 0b00110011, 0b01010101 }));
        reader.get(1);
        REQUIRE(reader.get(32) == 0b1001100'10101010'11110000'00110011'0);
    }
}

TEST_CASE("Checking for EOF works", "[io]")
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

TEST_CASE("Checking size works", "[io]")
{
    BitReader reader1("\x00\x00"_b);
    REQUIRE(reader1.size() == 16);
    BitReader reader2("\x00"_b);
    REQUIRE(reader2.size() == 8);
    BitReader reader3(""_b);
    REQUIRE(reader3.size() == 0);
}

TEST_CASE("Seeking works", "[io]")
{
    SECTION("Integer aligned")
    {
        BitReader reader(from_bits({
            0b00000000, 0b00000000, 0b00000000, 0b00000000,
            0b11111111, 0b11111111, 0b11111111, 0b11111111,
            0b11001100, 0b10101010, 0b11110000, 0b00110011 }));
        reader.seek(0);
        REQUIRE(reader.get(32) == 0b00000000000000000000000000000000);
        reader.seek(32);
        REQUIRE(reader.get(32) == 0b11111111111111111111111111111111);
        reader.seek(64);
        REQUIRE(reader.get(32) == 0b11001100101010101111000000110011);
    }

    SECTION("Byte aligned")
    {
        BitReader reader(from_bits({
            0b11001100, 0b10101010, 0b11110000, 0b00110011 }));
        reader.seek(0);  REQUIRE(reader.get(8) == 0b11001100);
        reader.seek(8);  REQUIRE(reader.get(8) == 0b10101010);
        reader.seek(16); REQUIRE(reader.get(8) == 0b11110000);
        reader.seek(24); REQUIRE(reader.get(8) == 0b00110011);
    }

    SECTION("Unaligned")
    {
        BitReader reader(from_bits({
            0b11001100, 0b10101010, 0b11110000, 0b00110011 }));
        reader.seek(0);  REQUIRE(reader.get(8) == 0b11001100);
        reader.seek(1);  REQUIRE(reader.get(8) == 0b10011001);
        reader.seek(2);  REQUIRE(reader.get(8) == 0b00110010);
        reader.seek(3);  REQUIRE(reader.get(8) == 0b01100101);
        reader.seek(4);  REQUIRE(reader.get(8) == 0b11001010);
        reader.seek(5);  REQUIRE(reader.get(8) == 0b10010101);
        reader.seek(6);  REQUIRE(reader.get(8) == 0b00101010);
        reader.seek(7);  REQUIRE(reader.get(8) == 0b01010101);
        reader.seek(8);  REQUIRE(reader.get(8) == 0b10101010);
        reader.seek(9);  REQUIRE(reader.get(8) == 0b01010101);
        reader.seek(10); REQUIRE(reader.get(8) == 0b10101011);
        reader.seek(11); REQUIRE(reader.get(8) == 0b01010111);
        reader.seek(12); REQUIRE(reader.get(8) == 0b10101111);
        reader.seek(13); REQUIRE(reader.get(8) == 0b01011110);
        reader.seek(14); REQUIRE(reader.get(8) == 0b10111100);
        reader.seek(15); REQUIRE(reader.get(8) == 0b01111000);
        reader.seek(16); REQUIRE(reader.get(8) == 0b11110000);
        reader.seek(17); REQUIRE(reader.get(8) == 0b11100000);
        reader.seek(18); REQUIRE(reader.get(8) == 0b11000000);
        reader.seek(19); REQUIRE(reader.get(8) == 0b10000001);
        reader.seek(20); REQUIRE(reader.get(8) == 0b00000011);
        reader.seek(21); REQUIRE(reader.get(8) == 0b00000110);
        reader.seek(22); REQUIRE(reader.get(8) == 0b00001100);
        reader.seek(23); REQUIRE(reader.get(8) == 0b00011001);
        reader.seek(24); REQUIRE(reader.get(8) == 0b00110011);
    }

    SECTION("Unaligned (automatic)")
    {
        BitReader reader(from_bits({
            0b11001100, 0b10101010, 0b11110000, 0b00110011 }));
        for (auto i : util::range(32))
        {
            reader.seek(i);
            INFO("Position: " << reader.tell());
            auto mask = (1ul << (32 - i)) - 1;
            auto expected = 0b11001100101010101111000000110011 & mask;
            REQUIRE(reader.get(32 - i) == expected);
        }
    }

    SECTION("Seeking beyond EOF throws errors")
    {
        BitReader reader(from_bits({
            0b11001100, 0b10101010, 0b11110000, 0b00110011 }));
        for (auto i : util::range(32))
        {
            reader.seek(31);
            REQUIRE_THROWS(reader.skip(2 + i));
            REQUIRE(reader.tell() == 31);
        }
        for (auto i : util::range(32))
            REQUIRE_THROWS(reader.seek(33 + i));
    }
}

TEST_CASE("Skipping works", "[io]")
{
    BitReader reader(from_bits({
        0b11001100, 0b10101010, 0b11110000, 0b00110011 }));
    reader.seek(0);
    REQUIRE(reader.get(8) == 0b11001100);
    reader.skip(-7);
    REQUIRE(reader.get(8) == 0b10011001);
}

TEST_CASE("Reading beyond EOF retracts to prior offset", "[io]")
{
    SECTION("Byte-aligned without byte retrieval")
    {
        BitReader reader("\x00"_b);
        reader.get(7);
        reader.get(1);
        REQUIRE(reader.eof());
        REQUIRE_THROWS(reader.get(1));
        REQUIRE(reader.eof());
        REQUIRE(reader.tell() == 8);
    }

    SECTION("Byte-aligned with byte retrieval")
    {
        BitReader reader("\x00\xFF"_b);
        reader.get(7);
        reader.get(1);
        REQUIRE_THROWS(reader.get(16));
        REQUIRE(!reader.eof());
        REQUIRE(reader.tell() == 8);
        REQUIRE(reader.get(8) == 0xFF);
    }

    SECTION("Byte-unaligned without byte retrieval")
    {
        BitReader reader("\x01"_b);
        reader.get(7);
        REQUIRE_THROWS(reader.get(2));
        REQUIRE(!reader.eof());
        REQUIRE(reader.tell() == 7);
        REQUIRE(reader.get(1));
    }

    SECTION("Byte-unaligned with byte retrieval")
    {
        BitReader reader("\x01\x00"_b);
        reader.get(7);
        REQUIRE_THROWS(reader.get(10));
        REQUIRE(!reader.eof());
        REQUIRE(reader.tell() == 7);
        REQUIRE(reader.get(1));
    }
}
