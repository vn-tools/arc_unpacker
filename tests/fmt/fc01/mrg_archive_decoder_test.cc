#include "fmt/fc01/mrg_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::fc01;

static const std::string dir = "tests/fmt/fc01/files/mrg/";

static void do_test(const std::string &input_path)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("123.txt", "123123123 123123123 321"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };
    const auto decoder = MrgArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("FC01 MRG archives", "[fmt]")
{
    SECTION("Plain")
    {
        do_test("plain.mrg");
    }

    SECTION("Compressed")
    {
        do_test("compressed.mrg");
    }
}
