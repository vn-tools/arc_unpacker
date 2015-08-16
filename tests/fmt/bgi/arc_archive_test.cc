#include "fmt/bgi/arc_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::bgi;

TEST_CASE("Unpacking ARC archives works")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::create_file("abc.txt", "123"_b),
        tests::create_file("another.txt", "abcdefghij"_b),
    };

    std::string path = "tests/fmt/bgi/files/test.arc";
    ArcArchive archive;
    tests::compare_files(
        expected_files, tests::unpack_to_memory(path, archive), true);
}
