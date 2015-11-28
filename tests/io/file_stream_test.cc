#include "io/file_stream.h"
#include "io/filesystem.h"
#include "test_support/catch.hh"
#include "test_support/stream_test.h"

using namespace au;
using namespace au::io;

TEST_CASE("FileStream", "[io][stream]")
{
    SECTION("Reading from existing files")
    {
        static const bstr png_magic = "\x89PNG"_b;
        FileStream stream(
            "tests/fmt/png/files/reimu_transparent.png", FileMode::Read);
        REQUIRE(stream.read(png_magic.size()) == png_magic);
    }

    SECTION("Creating to files")
    {
        REQUIRE(!io::exists("tests/trash.out"));

        {
            FileStream stream("tests/trash.out", FileMode::Write);
            REQUIRE_NOTHROW(stream.write_u32_le(1));
        }

        {
            FileStream stream("tests/trash.out", FileMode::Read);
            REQUIRE(stream.read_u32_le() == 1);
            REQUIRE(stream.size() == 4);
        }

        io::remove("tests/trash.out");
    }

    tests::stream_test(
        []()
        {
            return std::make_unique<FileStream>(
                "tests/trash.out", FileMode::Write);
        },
        []()
        {
            io::remove("tests/trash.out");
        });
}
