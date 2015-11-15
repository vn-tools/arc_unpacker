#include "fmt/kirikiri/xp3_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::kirikiri;

static const std::string dir = "tests/fmt/kirikiri/files/xp3/";

static void do_test(const std::string &input_path)
{
    const std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.txt", "123"_b),
        tests::stub_file("abc2.txt", "AAAAAAAAAA"_b),
    };
    Xp3ArchiveDecoder decoder;
    decoder.set_plugin("noop");
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("KiriKiri XP3 archives", "[fmt]")
{
    SECTION("Version 1")
    {
        do_test("xp3-v1.xp3");
    }

    SECTION("Version 2")
    {
        do_test("xp3-v2.xp3");
    }

    SECTION("Compressed file table")
    {
        do_test("xp3-compressed-table.xp3");
    }

    SECTION("Compressed file data")
    {
        do_test("xp3-compressed-files.xp3");
    }

    SECTION("Multiple SEGM chunks")
    {
        do_test("xp3-multiple-segm.xp3");
    }
}
