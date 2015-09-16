#include "fmt/nsystem/fjsys_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::nsystem;

TEST_CASE("NSystem FJSYS archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.txt", "123"_b),
        tests::stub_file("another.txt", "abcdefghij"_b),
    };

    FjsysArchive archive;
    auto actual_files = tests::unpack_to_memory(
        "tests/fmt/nsystem/files/fjsys/test.fjsys", archive);

    tests::compare_files(expected_files, actual_files, true);
}
