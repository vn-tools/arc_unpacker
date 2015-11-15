#include "fmt/team_shanghai_alice/pbg4_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::team_shanghai_alice;

TEST_CASE("Team Shanghai Alice PBG4 archives", "[fmt]")
{
    const std::vector<std::shared_ptr<File>> expected_files
    {
        tests::stub_file("abc.txt", "123"_b),
        tests::stub_file("another.txt", "abcdefghijaaabcd"_b),
    };

    const Pbg4ArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/team_shanghai_alice/files/pbg4/test.pbg4");
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(expected_files, actual_files, true);
}
