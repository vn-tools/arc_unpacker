#include "fmt/tactics/arc_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::tactics;

static const std::string dir = "tests/fmt/tactics/files/arc/";

static void do_test(const std::string &input_path)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("123.txt", "123123123123123123123"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };
    const auto decoder = ArcArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("TACTICS ARC archives", "[fmt]")
{
    SECTION("Version 0, compressed")
    {
        do_test("v0-compressed.arc");
    }

    SECTION("Version 0, uncompressed")
    {
        do_test("v0-uncompressed.arc");
    }

    SECTION("Version 1, uncompressed")
    {
        do_test("v1-uncompressed.arc");
    }

    SECTION("Version 1, compressed")
    {
        do_test("v1-compressed.arc");
    }
}
