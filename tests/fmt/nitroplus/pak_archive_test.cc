#include "fmt/nitroplus/pak_archive.h"
#include "test_support/archive_support.h"
#include "test_support/file_support.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt::nitroplus;

static void test_pak_archive(const std::string &path)
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.txt", "123"_b),
        tests::stub_file("another.txt", "abcdefghij"_b),
    };

    PakArchive archive;
    tests::compare_files(
        expected_files, au::tests::unpack_to_memory(path, archive), true);
}

TEST_CASE("Unpacking uncompressed PAK archives works")
{
    test_pak_archive("tests/fmt/nitroplus/files/pak/uncompressed.pak");
}

TEST_CASE("Unpacking compressed PAK archives works")
{
    test_pak_archive("tests/fmt/nitroplus/files/pak/compressed.pak");
}
