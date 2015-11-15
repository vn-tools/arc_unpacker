#include "fmt/rpgmaker/rgssad_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::rpgmaker;

TEST_CASE("RpgMaker RGSSAD archives", "[fmt]")
{
    const std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.txt", "123"_b),
        tests::stub_file("another.txt", "abcdefghij"_b),
    };

    const RgssadArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/rpgmaker/files/rgssad/test.rgssad");
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
