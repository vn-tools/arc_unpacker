#include "fmt/playstation/gpda_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::playstation;

static const std::string dir = "tests/fmt/playstation/files/gpda/";

static void do_test(
    const std::string &input_path,
    const std::initializer_list<std::shared_ptr<io::File>> &expected_files)
{
    const auto decoder = GpdaArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Playstation GPDA archives", "[fmt]")
{
    SECTION("Plain")
    {
        do_test(
            "test.dat",
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }

    SECTION("Nameless files")
    {
        do_test(
            "test.dat",
            {
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }
}
