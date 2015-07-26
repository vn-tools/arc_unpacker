#include "fmt/kirikiri/xp3_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hpp"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::kirikiri;

static void test_xp3_archive(const std::string &path)
{
    std::shared_ptr<File> file1(new File);
    std::shared_ptr<File> file2(new File);
    file1->name = "abc.txt";
    file2->name = "abc2.txt";
    file1->io.write("123", 3);
    file2->io.write("AAAAAAAAAA", 10);
    std::vector<std::shared_ptr<File>> expected_files { file1, file2 };

    std::unique_ptr<Archive> archive(new Xp3Archive);
    au::tests::compare_files(
        expected_files, au::tests::unpack_to_memory(path, *archive));
}

TEST_CASE("Unpacking version 1 XP3 archives works")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3-v1.xp3");
}

TEST_CASE("Unpacking version 2 XP3 archives works")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3-v2.xp3");
}

TEST_CASE("Unpacking XP3 archives with compressed file table works")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3-compressed-table.xp3");
}

TEST_CASE("Unpacking XP3 archives with compressed file data works")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3-compressed-files.xp3");
}

TEST_CASE("Unpacking XP3 archives with multiple SEGM chunks works")
{
    test_xp3_archive("tests/fmt/kirikiri/files/xp3-multiple-segm.xp3");
}
