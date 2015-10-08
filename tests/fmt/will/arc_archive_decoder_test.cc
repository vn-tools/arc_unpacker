#include "fmt/will/arc_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::will;

static void do_test(const std::string &input_path)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("!@#.txt", "!@#$%^&*()"_b),
    };

    ArcArchiveDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Will Co. ARC archives (9 character long names)", "[fmt]")
{
    do_test("tests/fmt/will/files/arc/names-9.arc");
}

TEST_CASE("Will Co. ARC archives (13 character long names)", "[fmt]")
{
    do_test("tests/fmt/will/files/arc/names-13.arc");
}

TEST_CASE("Will Co. ARC archives (script encryption)", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.SCR", "1234567890"_b),
        tests::stub_file("abc.WSC", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    ArcArchiveDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/will/files/arc/scripts.arc");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
