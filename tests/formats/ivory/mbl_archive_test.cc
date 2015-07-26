#include "formats/ivory/mbl_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hpp"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::ivory;

static void test_mbl_archive(const std::string &path)
{
    std::shared_ptr<File> file1(new File);
    std::shared_ptr<File> file2(new File);
    file1->name = "abc.txt";
    file2->name = "テスト";
    file1->io.write("abc", 3);
    file2->io.write("AAAAAAAAAAAAAAAA", 16);
    std::vector<std::shared_ptr<File>> expected_files { file1, file2 };

    std::unique_ptr<Archive> archive(new MblArchive);
    au::tests::compare_files(
        expected_files, au::tests::unpack_to_memory(path, *archive));
}

TEST_CASE("Unpacking version 1 MBL archives works")
{
    test_mbl_archive("tests/formats/ivory/files/mbl-v1.mbl");
}

TEST_CASE("Unpacking version 2 MBL archives works")
{
    test_mbl_archive("tests/formats/ivory/files/mbl-v2.mbl");
}
