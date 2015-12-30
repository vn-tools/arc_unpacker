#include "fmt/renpy/rpa_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::renpy;

static const std::string dir = "tests/fmt/renpy/files/rpa/";

static void test(const std::string &path)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("another.txt", "abcdefghij"_b),
        tests::stub_file("abc.txt", "123"_b),
    };
    const auto decoder = RpaArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Ren'py RPA archives", "[fmt]")
{
    SECTION("Version 3")
    {
        test("v3.rpa");
    }

    SECTION("Version 2")
    {
        test("v2.rpa");
    }

    SECTION("Data prefixes")
    {
        test("prefixes.rpa");
    }
}
