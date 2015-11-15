#include "fmt/minato_soft/pac_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::minato_soft;

static void do_test(const std::string &input_path)
{
    const std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    const PacArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("MinatoSoft PAC uncompressed archives", "[fmt]")
{
    do_test("tests/fmt/minato_soft/files/pac/uncompressed.pac");
}

TEST_CASE("MinatoSoft PAC compressed archives", "[fmt]")
{
    do_test("tests/fmt/minato_soft/files/pac/compressed.pac");
}
