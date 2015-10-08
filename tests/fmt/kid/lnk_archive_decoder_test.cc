#include "fmt/kid/lnk_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::kid;

static void do_test(const std::string &input_path)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.xyz", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    LnkArchiveDecoder decoder;
    auto actual_files = tests::unpack_to_memory(input_path, decoder);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("Kid LNK uncompressed unencrypted archives", "[fmt]")
{
    do_test("tests/fmt/kid/files/lnk/uncompressed.lnk");
}

TEST_CASE("Kid LNK compressed unencrypted archives", "[fmt]")
{
    do_test("tests/fmt/kid/files/lnk/compressed.lnk");
}

TEST_CASE("Kid LNK uncompressed encrypted archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("audio.wav", bstr(0x2000, '\xFF')),
        tests::stub_file("image.jpg", bstr(0x2000, '\xFF')),
        tests::stub_file("screensaver.scr", bstr(0x2000, '\xFF')),
    };

    LnkArchiveDecoder decoder;
    auto actual_files = decoder.unpack(*tests::zlib_file_from_path(
        "tests/fmt/kid/files/lnk/encrypted.lnk"));
    tests::compare_files(expected_files, actual_files, true);
}
