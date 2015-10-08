#include "fmt/wild_bug/wbp_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::wild_bug;

TEST_CASE("Wild Bug WBP archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("\\123.txt", "1234567890"_b),
        tests::stub_file("\\derp\\abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
        tests::stub_file("\\derp\\!@#.txt", "!@#$%^&*()_+"_b),
    };

    WbpArchiveDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/wild_bug/files/wbp/test.wbp");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
