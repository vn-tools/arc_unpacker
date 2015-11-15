#include "fmt/will/arc_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::will;

static const std::string dir = "tests/fmt/will/files/arc/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::shared_ptr<File>> &expected_files)
{
    const ArcArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Will Co. ARC archives", "[fmt]")
{
    SECTION("9 character long names")
    {
        do_test(
            "names-9.arc",
            {
                tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("!@#.txt", "!@#$%^&*()"_b),
            });
    }

    SECTION("13 character long names")
    {
        do_test(
            "names-13.arc",
            {
                tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
                tests::stub_file("123.txt", "1234567890"_b),
                tests::stub_file("!@#.txt", "!@#$%^&*()"_b),
            });
    }

    SECTION("Script encryption")
    {
        do_test(
            "scripts.arc",
            {
                tests::stub_file("123.SCR", "1234567890"_b),
                tests::stub_file("abc.WSC", "abcdefghijklmnopqrstuvwxyz"_b),
            });
    }
}
