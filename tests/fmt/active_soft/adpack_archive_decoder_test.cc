#include "fmt/active_soft/adpack_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::active_soft;

TEST_CASE("ActiveSoft ADPACK32 archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    AdpackArchiveDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/active_soft/files/adpack/test.pak");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
