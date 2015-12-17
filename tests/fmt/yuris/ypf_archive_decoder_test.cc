#include "fmt/yuris/ypf_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::yuris;

static const std::string dir = "tests/fmt/yuris/files/ypf/";

static void do_test(const std::string &input_path)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
    };
    const YpfArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("YuRis YPF archives", "[fmt]")
{
    SECTION("Plain")
    {
        do_test("compressed.ypf");
    }

    SECTION("Compressed")
    {
        do_test("compressed.ypf");
    }

    SECTION("Encrypted")
    {
        do_test("encrypted.ypf");
    }

    SECTION("Version 222")
    {
        do_test("v222.ypf");
    }

    SECTION("Version 300")
    {
        do_test("v300.ypf");
    }
}
