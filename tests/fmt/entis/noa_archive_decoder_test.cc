#include "fmt/entis/noa_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::entis;

TEST_CASE("Entis NOA unencrypted archives", "[fmt]")
{
    const std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    const NoaArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/entis/files/noa/unencrypted.noa");
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
