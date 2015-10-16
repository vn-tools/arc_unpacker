#include "fmt/amuse_craft/pac_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::amuse_craft;

TEST_CASE("Amuse Craft PAC archives (v1)", "[fmt]")
{
    const std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    PacArchiveDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/amuse_craft/files/pac/test-v1.pac");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Amuse Craft PAC archives (v2)", "[fmt]")
{
    const std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("dir/123.txt", "1234567890"_b),
        tests::stub_file("dir/abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    PacArchiveDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/amuse_craft/files/pac/test-v2.pac");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
