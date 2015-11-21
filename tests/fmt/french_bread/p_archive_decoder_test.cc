#include "fmt/french_bread/p_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::french_bread;

static const std::string dir = "tests/fmt/french_bread/files/p/";

static void do_test(const std::string &input_path)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
        tests::stub_file("big.000", bstr(0x3000, '\xFF')),
    };
    const PArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("French Bread P archives", "[fmt]")
{
    SECTION("Version 0")
    {
        do_test("test-v0.p");
    }

    SECTION("Version 1")
    {
        do_test("test-v1.p");
    }
}
