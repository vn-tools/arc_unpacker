#include "dec/eagls/pak_script_file_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::eagls;

static const std::string dir = "tests/dec/eagls/files/pak-script/";

static void do_test(
    const std::string &input_path,
    const std::string &expected_path)
{
    const auto decoder = PakScriptFileDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*actual_file, *expected_file, false);
}

TEST_CASE("EAGLS PAK scripts", "[dec]")
{
    do_test("00Init.dat", "00Init-out.txt");
    do_test("01Thread.dat", "01Thread-out.txt");
}
