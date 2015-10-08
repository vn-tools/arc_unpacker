#include "fmt/bgi/arc_archive_decoder.h"
#include "test_support/decoder_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::bgi;

TEST_CASE("BGI ARC archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.txt", "123"_b),
        tests::stub_file("another.txt", "abcdefghij"_b),
    };

    ArcArchiveDecoder decoder;
    auto input_file = tests::file_from_path("tests/fmt/bgi/files/arc/test.arc");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
