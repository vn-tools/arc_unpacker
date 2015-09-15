#include "fmt/eagls/pak_script_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::eagls;

static void do_test(
    const std::string &input_path,
    const std::string &expected_path)
{
    PakScriptConverter converter;
    auto input_file = tests::file_from_path(input_path);
    auto expected_file = tests::file_from_path(expected_path);
    auto actual_file = converter.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("Decoding EAGLS's PAK scripts works", "[fmt]")
{
    do_test(
        "tests/fmt/eagls/files/pak-script/00Init.dat",
        "tests/fmt/eagls/files/pak-script/00Init-out.txt");
    do_test(
        "tests/fmt/eagls/files/pak-script/01Thread.dat",
        "tests/fmt/eagls/files/pak-script/01Thread-out.txt");
}
