#include "fmt/alice_soft/dat_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

TEST_CASE("Alice Soft DAT archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("000.dat", "123"_b + bstr(253)),
        tests::stub_file("001.dat", "abcdefghijklmnopqrstuvwxyz"_b + bstr(230)),
    };

    DatArchiveDecoder decoder;
    auto actual_files = au::tests::unpack_to_memory(
        "tests/fmt/alice_soft/files/dat/test.dat", decoder);

    au::tests::compare_files(expected_files, actual_files, true);
}
