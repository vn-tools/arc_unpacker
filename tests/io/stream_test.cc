#include "io/file_stream.h"
#include "io/filesystem.h"
#include "io/memory_stream.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::io;

TEST_CASE("Reading real files", "[io][io_cls]")
{
    FileStream stream(
        "tests/fmt/png/files/reimu_transparent.png", FileMode::Read);
    static const bstr png_magic = "\x89PNG"_b;
    REQUIRE(stream.read(png_magic.size()) == png_magic);
}

TEST_CASE("Writing to real files", "[io][io_cls]")
{
    {
        FileStream stream("tests/trash.out", FileMode::Write);
        stream.write_u32_le(1);
        stream.seek(0);
        REQUIRE(stream.read_u32_le() == 1);
        REQUIRE(stream.size() == 4);
    }
    io::remove("tests/trash.out");
}

TEST_CASE("Proper file position after initialization", "[io][io_cls]")
{
    const MemoryStream stream;
    REQUIRE(stream.size() == 0);
    REQUIRE(stream.tell() == 0);
}

TEST_CASE("NULL bytes in binary data don't cause anomalies", "[io][io_cls]")
{
    MemoryStream stream("\x00\x00\x00\x01"_b);
    REQUIRE(stream.size() == 4);
    REQUIRE(stream.read_u32_le() == 0x01000000);
    stream.seek(0);
    stream.write("\x00\x00\x00\x02"_b);
    stream.seek(0);
    REQUIRE(stream.read_u32_le() == 0x02000000);
}

TEST_CASE("Reading integers", "[io][io_cls]")
{
    MemoryStream stream("\x01\x00\x00\x00"_b);
    stream.seek(0);
    REQUIRE(stream.read_u32_le() == 1);
    REQUIRE(stream.size() == 4);
}

TEST_CASE("Writing integers", "[io][io_cls]")
{
    MemoryStream stream;
    stream.write_u32_le(1);
    stream.seek(0);
    REQUIRE(stream.read_u32_le() == 1);
    REQUIRE(stream.size() == 4);
}

TEST_CASE("Skipping and telling position", "[io][io_cls]")
{
    MemoryStream stream("\x01\x0F\x00\x00"_b);

    SECTION("Positive offset")
    {
        stream.skip(1);
        REQUIRE(stream.tell() == 1);
        REQUIRE(stream.read_u16_le() == 15);
        REQUIRE(stream.tell() == 3);
    }

    SECTION("Negative offset")
    {
        stream.read(2);
        stream.skip(-1);
        REQUIRE(stream.read_u16_le() == 15);
        REQUIRE(stream.tell() == 3);
    }
}

TEST_CASE("Seeking and telling position", "[io][io_cls]")
{
    MemoryStream stream("\x01\x00\x00\x00"_b);

    SECTION("Initial seeking")
    {
        REQUIRE(stream.tell() == 0);
    }

    SECTION("Seeking to EOF")
    {
        stream.seek(4);
        REQUIRE(stream.tell() == 4);
    }

    SECTION("Seeking beyond EOF throws errors")
    {
        stream.seek(3);
        REQUIRE_THROWS(stream.seek(5));
        REQUIRE(stream.tell() == 3);
    }

    SECTION("Seeking affects what gets read")
    {
        stream.seek(1);
        REQUIRE(stream.read_u16_le() == 0);
        stream.seek(0);
        REQUIRE(stream.read_u32_le() == 1);
    }

    SECTION("Seeking affects telling position")
    {
        stream.seek(2);
        REQUIRE(stream.tell() == 2);
        stream.skip(1);
        REQUIRE(stream.tell() == 3);
        stream.skip(-1);
        REQUIRE(stream.tell() == 2);
    }
}

TEST_CASE("Reading NULL-terminated strings", "[io][io-cls]")
{
    MemoryStream stream("abc\x00""def\x00"_b);
    REQUIRE(stream.read_to_zero() == "abc"_b);
    REQUIRE(stream.read_to_zero() == "def"_b);
}

TEST_CASE("Reading lines", "[io][io_cls]")
{
    MemoryStream stream("line1\nline2\n"_b);
    REQUIRE(stream.read_line() == "line1"_b);
    REQUIRE(stream.read_line() == "line2"_b);
}

TEST_CASE("Reading unterminated lines", "[io][io_cls]")
{
    MemoryStream stream("line"_b);
    REQUIRE(stream.read_line() == "line"_b);
}

TEST_CASE("Reading NULL-terminated lines", "[io][io_cls]")
{
    MemoryStream stream("line\x00"_b);
    REQUIRE(stream.read_line() == "line"_b);
}

TEST_CASE("Reading lines containing carriage returns", "[io][io_cls]")
{
    MemoryStream stream("line1\r\nline2\r\n"_b);
    REQUIRE(stream.read_line() == "line1"_b);
    REQUIRE(stream.read_line() == "line2"_b);
}

TEST_CASE("Reading strings", "[io][io_cls]")
{
    MemoryStream stream("abc\x00"_b);
    const auto result = stream.read(2);
    REQUIRE(result == "ab"_b);
}

TEST_CASE("Writing strings", "[io][io_cls]")
{
    MemoryStream stream("abc\x00"_b);
    stream.write("xy"_b);
    stream.skip(-2);
    const auto result = stream.read(3);
    REQUIRE(result == "xyc"_b);
}

TEST_CASE("Reading integers with endianness conversions", "[io][io_cls]")
{
    MemoryStream stream("\x12\x34\x56\x78"_b);
    REQUIRE(stream.read_u8() == 0x12); stream.skip(-1);
    REQUIRE(stream.read_u16_le() == 0x3412); stream.skip(-2);
    REQUIRE(stream.read_u16_be() == 0x1234); stream.skip(-2);
    REQUIRE(stream.read_u32_le() == 0x78563412); stream.skip(-4);
    REQUIRE(stream.read_u32_be() == 0x12345678); stream.skip(-4);
}
