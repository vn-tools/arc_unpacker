#include "fmt/sysadv/pak_archive.h"
#include "test_support/archive_support.h"
#include "test_support/file_support.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt::sysadv;

TEST_CASE("Unpacking sysadv's PAK archives works")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    PakArchive archive;
    auto actual_files = au::tests::unpack_to_memory(
        "tests/fmt/sysadv/files/pak/test.pak", archive);

    tests::compare_files(expected_files, actual_files, true);
}
