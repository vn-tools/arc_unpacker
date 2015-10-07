#include "fmt/alice_soft/alk_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

TEST_CASE("Alice Soft ALK archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("000.dat", "1234567890"_b),
        tests::stub_file("001.dat", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    AlkArchiveDecoder decoder;
    auto actual_files = au::tests::unpack_to_memory(
        "tests/fmt/alice_soft/files/alk/test.alk", decoder);

    au::tests::compare_files(expected_files, actual_files, true);
}
