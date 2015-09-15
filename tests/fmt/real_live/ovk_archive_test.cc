#include "fmt/real_live/ovk_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::real_live;

TEST_CASE("Unpacking RealLive's OVK archives works", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("sample00010", "1234567890"_b),
        tests::stub_file("sample00025", "abcdefghijklmnopqrstuvwxyz"_b),
    };

    OvkArchive archive;
    auto actual_files = au::tests::unpack_to_memory(
        "tests/fmt/real_live/files/ovk/test.ovk", archive);

    tests::compare_files(expected_files, actual_files, true);
}
