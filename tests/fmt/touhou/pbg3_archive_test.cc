#include "fmt/touhou/pbg3_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::touhou;

TEST_CASE("Unpacking PBG3 archives works")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::create_file("abc.txt", "123"_b),
        tests::create_file("another.txt", "abcdefghijaaabcd"_b),
    };

    Pbg3Archive archive;
    tests::compare_files(
        expected_files,
        tests::unpack_to_memory("tests/fmt/touhou/files/test.pbg3", archive),
        true);
}
