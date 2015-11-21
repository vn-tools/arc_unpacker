#include "fmt/amuse_craft/pac_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::amuse_craft;

static const std::string dir = "tests/fmt/amuse_craft/files/pac/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<io::File>> &expected_files)
{
    const PacArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Amuse Craft PAC archives", "[fmt]")
{
    SECTION("Version 1")
    {
        do_test(
            "test-v1.pac",
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }

    SECTION("Version 2")
    {
        do_test(
            "test-v2.pac",
            {
                tests::stub_file("dir/123.txt", "1234567890"_b),
                tests::stub_file("dir/abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }
}
