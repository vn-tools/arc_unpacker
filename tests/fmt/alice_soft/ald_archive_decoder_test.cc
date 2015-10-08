#include "fmt/alice_soft/ald_archive_decoder.h"
#include "test_support/decoder_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

TEST_CASE("Alice Soft ALD archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    AldArchiveDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/alice_soft/files/ald/test.ald");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
