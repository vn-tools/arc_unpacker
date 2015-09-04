#include "fmt/alice_soft/ald_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

TEST_CASE("Unpacking AliceSoft's ALD archives works")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    AldArchive archive;
    auto actual_files = au::tests::unpack_to_memory(
        "tests/fmt/alice_soft/files/ald/test.ald", archive);

    au::tests::compare_files(expected_files, actual_files, true);
}
