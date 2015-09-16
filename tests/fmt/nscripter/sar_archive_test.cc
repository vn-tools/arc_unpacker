#include "fmt/nscripter/sar_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::nscripter;

TEST_CASE("NScripter SAR archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.txt", "123"_b),
        tests::stub_file("dir/another.txt", "AAAAAAAAAAAAAAAA"_b),
    };

    SarArchive archive;
    auto actual_files = tests::unpack_to_memory(
        "tests/fmt/nscripter/files/sar/test.sar", archive);

    tests::compare_files(expected_files, actual_files, true);
}
