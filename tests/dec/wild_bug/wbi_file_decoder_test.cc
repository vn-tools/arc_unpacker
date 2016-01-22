#include "dec/wild_bug/wbi_file_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::wild_bug;

static const std::string dir = "tests/dec/wild_bug/files/wbi/";

static void do_test(
    const std::string &input_path,
    const std::string &expected_path)
{
    const auto decoder = WbiFileDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*actual_file, *expected_file, false);
}

TEST_CASE("Wild Bug WBI script files", "[dec]")
{
    do_test("START.WBI", "START-out.dat");
}
