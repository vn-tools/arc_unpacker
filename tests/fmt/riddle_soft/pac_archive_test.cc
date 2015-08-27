#include "fmt/riddle_soft/pac_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::riddle_soft;

TEST_CASE("Unpacking Riddle Soft's PAC archives works")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    PacArchive archive;
    auto actual_files = tests::unpack_to_memory(
        "tests/fmt/riddle_soft/files/pac/test.pac", archive);

    tests::compare_files(expected_files, actual_files, true);
}
