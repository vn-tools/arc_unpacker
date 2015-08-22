#include "fmt/ivory/mbl_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::ivory;

static void test_mbl_archive(const std::string &path)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::create_file("abc.txt", "abc"_b),
        tests::create_file("テスト", "AAAAAAAAAAAAAAAA"_b),
    };

    MblArchive archive;
    tests::compare_files(
        expected_files, au::tests::unpack_to_memory(path, archive), true);
}

TEST_CASE("Unpacking version 1 MBL archives works")
{
    test_mbl_archive("tests/fmt/ivory/files/mbl/mbl-v1.mbl");
}

TEST_CASE("Unpacking version 2 MBL archives works")
{
    test_mbl_archive("tests/fmt/ivory/files/mbl/mbl-v2.mbl");
}
