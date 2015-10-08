#include "fmt/cherry_soft/myk_archive_decoder.h"
#include "test_support/decoder_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::cherry_soft;

TEST_CASE("CherrySoft MYK archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "123"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    MykArchiveDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/cherry_soft/files/myk/test.myk");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
