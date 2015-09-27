#include "fmt/gs/dat_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::gs;

TEST_CASE("GS DAT archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("00000.dat", "1234567890"_b),
        tests::stub_file("00001.dat", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    DatArchiveDecoder decoder;
    auto actual_files = tests::unpack_to_memory(
        "tests/fmt/gs/files/dat/test.dat", decoder);
    tests::compare_files(expected_files, actual_files, true);
}
