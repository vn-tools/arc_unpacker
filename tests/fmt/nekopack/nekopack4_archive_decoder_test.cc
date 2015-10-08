#include "fmt/nekopack/nekopack4_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::nekopack;

TEST_CASE("Nekopack Nekopack4 archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    Nekopack4ArchiveDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/nekopack/files/pak/nekopack4a.pak");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
