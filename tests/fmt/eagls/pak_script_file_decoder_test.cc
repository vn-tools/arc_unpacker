#include "fmt/eagls/pak_script_file_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::eagls;

static void do_test(
    const std::string &input_path,
    const std::string &expected_path)
{
    PakScriptFileDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_file = tests::file_from_path(expected_path);
    auto actual_file = decoder.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("EAGLS PAK scripts", "[fmt]")
{
    do_test(
        "tests/fmt/eagls/files/pak-script/00Init.dat",
        "tests/fmt/eagls/files/pak-script/00Init-out.txt");
    do_test(
        "tests/fmt/eagls/files/pak-script/01Thread.dat",
        "tests/fmt/eagls/files/pak-script/01Thread-out.txt");
}
