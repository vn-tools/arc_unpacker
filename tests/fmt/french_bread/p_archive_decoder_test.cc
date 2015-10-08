#include "fmt/french_bread/p_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::french_bread;

static void do_test(const std::string &input_path)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
        tests::stub_file("big.000", bstr(0x3000, '\xFF')),
    };

    PArchiveDecoder decoder;
    auto actual_files = tests::unpack_to_memory(input_path, decoder);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("French Bread P v0 archives", "[fmt]")
{
    do_test("tests/fmt/french_bread/files/p/test-v0.p");
}

TEST_CASE("French Bread P v1 archives", "[fmt]")
{
    do_test("tests/fmt/french_bread/files/p/test-v1.p");
}


