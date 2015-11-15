#include "fmt/liar_soft/xfl_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::liar_soft;

TEST_CASE("LiarSoft XFL archives", "[fmt]")
{
    const std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.txt", "123"_b),
        tests::stub_file("another.txt", "abcdefghij"_b),
    };

    const XflArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/liar_soft/files/xfl/test.xfl");
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
