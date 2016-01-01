#include "dec/yumemiru/dat_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::yumemiru;

static const std::string dir = "tests/dec/yumemiru/files/dat/";

static void do_test(const std::string &input_path)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
    };
    const auto decoder = DatArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Yumemiru DAT archives", "[dec]")
{
    SECTION("Yumemiru variant")
    {
        do_test("test.dat");
    }

    SECTION("EEE variant")
    {
        do_test("test.cab");
    }
}
