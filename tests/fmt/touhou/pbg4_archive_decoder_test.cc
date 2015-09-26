#include "fmt/touhou/pbg4_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::touhou;

TEST_CASE("Touhou PBG4 archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.txt", "123"_b),
        tests::stub_file("another.txt", "abcdefghijaaabcd"_b),
    };

    Pbg4ArchiveDecoder decoder;
    auto actual_files = tests::unpack_to_memory(
        "tests/fmt/touhou/files/pbg4/test.pbg4", decoder);

    tests::compare_files(expected_files, actual_files, true);
}
