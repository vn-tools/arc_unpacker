#include "dec/team_shanghai_alice/pbg3_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::team_shanghai_alice;

static const std::string dir = "tests/dec/team_shanghai_alice/files/pbg3/";

static void do_test(const std::string &input_path)
{
    const std::vector<std::shared_ptr<io::File>> expected_files
    {
        tests::stub_file("abc.txt", "123"_b),
        tests::stub_file("another.txt", "abcdefghijaaabcd"_b),
    };
    const auto decoder = Pbg3ArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    tests::compare_files(actual_files, expected_files, true);
}

TEST_CASE("Team Shanghai Alice PBG3 archives", "[dec]")
{
    do_test("test.pbg3");
}
