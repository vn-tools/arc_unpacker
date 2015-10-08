#include "fmt/tactics/arc_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::tactics;

static void do_test(const std::string &input_path)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "123123123123123123123"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    ArcArchiveDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("TACTICS ARC v0 compressed archives", "[fmt]")
{
    do_test("tests/fmt/tactics/files/arc/v0-compressed.arc");
}

TEST_CASE("TACTICS ARC v0 uncompressed archives", "[fmt]")
{
    do_test("tests/fmt/tactics/files/arc/v0-uncompressed.arc");
}

TEST_CASE("TACTICS ARC v1 uncompressed archives", "[fmt]")
{
    do_test("tests/fmt/tactics/files/arc/v1-uncompressed.arc");
}

TEST_CASE("TACTICS ARC v1 compressed archives", "[fmt]")
{
    do_test("tests/fmt/tactics/files/arc/v1-compressed.arc");
}
