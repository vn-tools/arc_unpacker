#include "fmt/eagls/pak_script_file_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::eagls;

static const std::string dir = "tests/fmt/eagls/files/pak-script/";

static void do_test(
    const std::string &input_path,
    const std::string &expected_path)
{
    const PakScriptFileDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("EAGLS PAK scripts", "[fmt]")
{
    do_test("00Init.dat", "00Init-out.txt");
    do_test("01Thread.dat", "01Thread-out.txt");
}
