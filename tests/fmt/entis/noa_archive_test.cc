#include "fmt/entis/noa_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::entis;

TEST_CASE("Entis NOA unencrypted archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    NoaArchive archive;
    auto actual_files = tests::unpack_to_memory(
        "tests/fmt/entis/files/noa/unencrypted.noa", archive);
    tests::compare_files(expected_files, actual_files, true);
}
