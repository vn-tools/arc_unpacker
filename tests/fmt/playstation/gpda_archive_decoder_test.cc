#include "fmt/playstation/gpda_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::playstation;

static void do_test(const std::string &input_path)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
    };
    GpdaArchiveDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Playstation GPDA archives", "[fmt]")
{
    do_test("tests/fmt/playstation/files/gpda/test.dat");
}
