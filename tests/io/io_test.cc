#include <boost/filesystem.hpp>
#include "io/file_io.h"
#include "io/buffered_io.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::io;

TEST_CASE("Reading real files works", "[io]")
{
    FileIO io("tests/files/reimu_transparent.png", FileMode::Read);
    static const bstr png_magic = "\x89PNG"_b;
    REQUIRE(io.read(png_magic.size()) == png_magic);
}

TEST_CASE("Writing to real files works", "[io]")
{
    {
        FileIO io("tests/files/trash.out", FileMode::Write);
        io.write_u32_le(1);
        io.seek(0);
        REQUIRE(io.read_u32_le() == 1);
        REQUIRE(io.size() == 4);
    }
    boost::filesystem::remove("tests/files/trash.out");
}

TEST_CASE("Initial IO has proper data", "[io]")
{
    BufferedIO io;
    REQUIRE(io.size() == 0);
    REQUIRE(io.tell() == 0);
}

TEST_CASE("NULL bytes in binary data don't cause anomalies", "[io]")
{
    BufferedIO io("\x00\x00\x00\x01"_b);
    REQUIRE(io.size() == 4);
    REQUIRE(io.read_u32_le() == 0x01000000);
    io.seek(0);
    io.write("\x00\x00\x00\x02"_b);
    io.seek(0);
    REQUIRE(io.read_u32_le() == 0x02000000);
}

TEST_CASE("Reading integers works", "[io]")
{
    BufferedIO io("\x01\x00\x00\x00"_b);
    REQUIRE(io.read_u32_le() == 1);
    REQUIRE(io.size() == 4);
}

TEST_CASE("Writing integers works", "[io]")
{
    BufferedIO io;
    io.write_u32_le(1);
    io.seek(0);
    REQUIRE(io.read_u32_le() == 1);
    REQUIRE(io.size() == 4);
}

TEST_CASE("Skipping and telling position works", "[io]")
{
    BufferedIO io("\x01\x0F\x00\x00"_b);

    SECTION("Positive offset")
    {
        io.skip(1);
        REQUIRE(io.tell() == 1);
        REQUIRE(io.read_u16_le() == 15);
        REQUIRE(io.tell() == 3);
    }

    SECTION("Negative offset")
    {
        io.read(2);
        io.skip(-1);
        REQUIRE(io.read_u16_le() == 15);
        REQUIRE(io.tell() == 3);
    }
}

TEST_CASE("Seeking and telling position works", "[io]")
{
    BufferedIO io("\x01\x00\x00\x00"_b);

    SECTION("Initial seeking")
    {
        REQUIRE(io.tell() == 0);
    }

    SECTION("Seeking to EOF")
    {
        io.seek(4);
        REQUIRE(io.tell() == 4);
    }

    SECTION("Seeking beyond EOF throws errors")
    {
        io.seek(3);
        REQUIRE_THROWS(io.seek(5));
        REQUIRE(io.tell() == 3);
    }

    SECTION("Seeking affects what gets read")
    {
        io.seek(1);
        REQUIRE(io.read_u16_le() == 0);
        io.seek(0);
        REQUIRE(io.read_u32_le() == 1);
    }

    SECTION("Seeking affects telling position")
    {
        io.seek(2);
        REQUIRE(io.tell() == 2);
        io.skip(1);
        REQUIRE(io.tell() == 3);
        io.skip(-1);
        REQUIRE(io.tell() == 2);
    }
}

TEST_CASE("Reading NULL-terminated strings works", "[io]")
{
    BufferedIO io("abc\x00""def\x00"_b);
    REQUIRE(io.read_to_zero() == "abc"_b);
    REQUIRE(io.read_to_zero() == "def"_b);
}

TEST_CASE("Reading lines works", "[io]")
{
    BufferedIO io("line1\nline2\n"_b);
    REQUIRE(io.read_line() == "line1"_b);
    REQUIRE(io.read_line() == "line2"_b);
}

TEST_CASE("Reading unterminated lines works", "[io]")
{
    BufferedIO io("line"_b);
    REQUIRE(io.read_line() == "line"_b);
}

TEST_CASE("Reading NULL-terminated lines works", "[io]")
{
    BufferedIO io("line\x00"_b);
    REQUIRE(io.read_line() == "line"_b);
}

TEST_CASE("Reading lines containing carriage returns works", "[io]")
{
    BufferedIO io("line1\r\nline2\r\n"_b);
    REQUIRE(io.read_line() == "line1"_b);
    REQUIRE(io.read_line() == "line2"_b);
}

TEST_CASE("Reading strings works", "[io]")
{
    BufferedIO io("abc\x00"_b);
    char result[2];
    io.read(result, 2);
    REQUIRE(memcmp("ab", result, 2) == 0);
}

TEST_CASE("Writing strings works", "[io]")
{
    BufferedIO io("abc\x00"_b);
    io.write("xy"_b);
    io.skip(-2);
    char result[3];
    io.read(result, 3);
    REQUIRE(memcmp("xyc", result, 3) == 0);
}

TEST_CASE("Reading integers with endianness conversions works", "[io]")
{
    BufferedIO io("\x12\x34\x56\x78"_b);
    REQUIRE(io.read_u8() == 0x12); io.skip(-1);
    REQUIRE(io.read_u16_le() == 0x3412); io.skip(-2);
    REQUIRE(io.read_u16_be() == 0x1234); io.skip(-2);
    REQUIRE(io.read_u32_le() == 0x78563412); io.skip(-4);
    REQUIRE(io.read_u32_be() == 0x12345678); io.skip(-4);
}
