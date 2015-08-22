#include "fmt/kirikiri/xp3_archive.h"
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

    Xp3Archive archive;
    tests::compare_files(
        expected_files, au::tests::unpack_to_memory(path, archive), true);
}

TEST_CASE("Unpacking version 1 XP3 archives works")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3/xp3-v1.xp3");
}

TEST_CASE("Unpacking version 2 XP3 archives works")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3/xp3-v2.xp3");
}

TEST_CASE("Unpacking XP3 archives with compressed file table works")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3/xp3-compressed-table.xp3");
}

TEST_CASE("Unpacking XP3 archives with compressed file data works")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3/xp3-compressed-files.xp3");
}

TEST_CASE("Unpacking XP3 archives with multiple SEGM chunks works")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3/xp3-multiple-segm.xp3");
}
