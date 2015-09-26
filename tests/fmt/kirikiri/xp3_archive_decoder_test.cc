#include "fmt/kirikiri/xp3_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/file_support.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt::kirikiri;

static void test_xp3_archive(const std::string &path)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.txt", "123"_b),
        tests::stub_file("abc2.txt", "AAAAAAAAAA"_b),
    };

    Xp3ArchiveDecoder decoder;
    auto actual_files = au::tests::unpack_to_memory(path, decoder);

    tests::compare_files(expected_files, actual_files, true);
}

TEST_CASE("KiriKiri XP3 v1 archives", "[fmt]")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3/xp3-v1.xp3");
}

TEST_CASE("KiriKiri XP3 v2 archives", "[fmt]")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3/xp3-v2.xp3");
}

TEST_CASE("KiriKiri XP3 archives with compressed file table", "[fmt]")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3/xp3-compressed-table.xp3");
}

TEST_CASE("KiriKiri XP3 archives with compressed file data", "[fmt]")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3/xp3-compressed-files.xp3");
}

TEST_CASE("KiriKiri XP3 archives with multiple SEGM chunks", "[fmt]")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3/xp3-multiple-segm.xp3");
}
