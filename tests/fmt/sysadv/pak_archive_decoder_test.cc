#include "fmt/sysadv/pak_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/file_support.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt::sysadv;

TEST_CASE("sysadv PAK archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("123.txt", "1234567890"_b),
        tests::stub_file("abc.txt", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    PakArchiveDecoder decoder;
    auto actual_files = au::tests::unpack_to_memory(
        "tests/fmt/sysadv/files/pak/test.pak", decoder);

    tests::compare_files(expected_files, actual_files, true);
}
