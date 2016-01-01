#include "dec/minato_soft/pac_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::minato_soft;

static const std::string dir = "tests/dec/minato_soft/files/pac/";

static void do_test(const std::string &input_path)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };
    const auto decoder = PacArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("MinatoSoft PAC archives", "[dec]")
{
    SECTION("Uncompressed")
    {
        do_test("uncompressed.pac");
    }

    SECTION("Compressed")
    {
        do_test("compressed.pac");
    }
}
