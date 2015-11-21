#include "fmt/alice_soft/dat_archive_decoder.h"
#include "test_support/decoder_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const std::string dir = "tests/fmt/alice_soft/files/dat/";

static void do_test(const std::string &input_path)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("000.dat", "123"_b + bstr(253)),
        tests::stub_file("001.dat", "abcdefghijklmnopqrstuvwxyz"_b + bstr(230)),
    };
    const DatArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Alice Soft DAT archives", "[fmt]")
{
    do_test("test.dat");
}
