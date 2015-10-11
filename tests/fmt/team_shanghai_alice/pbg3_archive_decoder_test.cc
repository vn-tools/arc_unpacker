#include "fmt/team_shanghai_alice/pbg3_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::team_shanghai_alice;

TEST_CASE("Team Shanghai Alice PBG3 archives", "[fmt]")
{
    std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.txt", "123"_b),
        tests::stub_file("another.txt", "abcdefghijaaabcd"_b),
    };

    Pbg3ArchiveDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/team_shanghai_alice/files/pbg3/test.pbg3");
    auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
