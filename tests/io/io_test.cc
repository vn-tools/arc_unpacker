#include <boost/filesystem.hpp>
#include "io/file_io.h"
#include "io/buffered_io.h"
#include "test_support/catch.hpp"

using namespace au::io;

TEST_CASE("Reading real files works")
{
    FileIO io("tests/files/reimu_transparent.png", FileMode::Read);
    const std::string png_magic = "\x89PNG"_s;
    REQUIRE(io.read(png_magic.size()) == png_magic);
}

TEST_CASE("Writing to real files works")
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

TEST_CASE("Initial IO has proper data")
{
    BufferedIO io;
    REQUIRE(io.size() == 0);
    REQUIRE(io.tell() == 0);
}

TEST_CASE("NULL bytes in binary data don't cause anomalies")
{
    BufferedIO io("\x00\x00\x00\x01"_s);
    REQUIRE(io.size() == 4);
    REQUIRE(io.read_u32_le() == 0x01000000);
    io.seek(0);
    io.write("\x00\x00\x00\x02"_s);
    io.seek(0);
    REQUIRE(io.read_u32_le() == 0x02000000);
}

TEST_CASE("Reading integers works")
{
    BufferedIO io("\x01\x00\x00\x00"_s);
    REQUIRE(io.read_u32_le() == 1);
    REQUIRE(io.size() == 4);
}

TEST_CASE("Writing integers works")
{
    BufferedIO io;
    io.write_u32_le(1);
    io.seek(0);
    REQUIRE(io.read_u32_le() == 1);
    REQUIRE(io.size() == 4);
}

TEST_CASE("Skipping and telling position works")
{
    BufferedIO io("\x01\x0F\x00\x00"_s);
    io.skip(1);
    REQUIRE(io.tell() == 1);
    REQUIRE(io.read_u16_le() == 15);
    REQUIRE(io.tell() == 3);
}

TEST_CASE("Seeking and telling position works")
{
    BufferedIO io("\x01\x00\x00\x00"_s);

    REQUIRE(io.tell() == 0);

    io.seek(4);
    REQUIRE(io.tell() == 4);

    try
    {
        io.seek(5);
        REQUIRE(0);
    }
    catch (...)
    {
        REQUIRE(io.tell() == 4);
    }

    io.seek(0);
    REQUIRE(io.read_u32_le() == 1);

    io.seek(0);
    REQUIRE(io.tell() == 0);
    REQUIRE(io.read_u32_le() == 1);
    io.seek(2);
    REQUIRE(io.tell() == 2);

    io.skip(1);
    REQUIRE(io.tell() == 3);

    io.skip(-1);
    REQUIRE(io.tell() == 2);
}

TEST_CASE("Reading NULL-terminated strings works")
{
    BufferedIO io("abc\x00""def\x00"_s);
    REQUIRE(io.read_until_zero() == "abc\x00");
    REQUIRE(io.read_until_zero() == "def\x00");
}

TEST_CASE("Reading lines works")
{
    BufferedIO io("line1\nline2\n"_s);
    REQUIRE(io.read_line() == "line1");
    REQUIRE(io.read_line() == "line2");
}

TEST_CASE("Reading unterminated lines works")
{
    BufferedIO io("line"_s);
    REQUIRE(io.read_line() == "line");
}

TEST_CASE("Reading NULL-terminated lines works")
{
    BufferedIO io("line\x00"_s);
    REQUIRE(io.read_line() == "line");
}

TEST_CASE("Reading lines containing carriage returns works")
{
    BufferedIO io("line1\r\nline2\r\n"_s);
    REQUIRE(io.read_line() == "line1");
    REQUIRE(io.read_line() == "line2");
}

TEST_CASE("Reading strings works")
{
    BufferedIO io("abc\x00"_s);
    char result[2];
    io.read(result, 2);
    REQUIRE(memcmp("ab", result, 2) == 0);
}

TEST_CASE("Writing strings works")
{
    BufferedIO io("abc\x00"_s);
    io.write("xy"_s);
    io.skip(-2);
    char result[3];
    io.read(result, 3);
    REQUIRE(memcmp("xyc", result, 3) == 0);
}

TEST_CASE("Reading integers with endianness conversions works")
{
    BufferedIO io("\x12\x34\x56\x78"_s);
    REQUIRE(io.read_u8() == 0x12); io.skip(-1);
    REQUIRE(io.read_u16_le() == 0x3412); io.skip(-2);
    REQUIRE(io.read_u16_be() == 0x1234); io.skip(-2);
    REQUIRE(io.read_u32_le() == 0x78563412); io.skip(-4);
    REQUIRE(io.read_u32_be() == 0x12345678); io.skip(-4);
}
