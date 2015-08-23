#include "fmt/ivory/wady_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::ivory;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    WadyConverter converter;
    auto input_file = tests::zlib_file_from_path(input_path);
    auto expected_file = tests::zlib_file_from_path(expected_path);
    auto actual_file = converter.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("Decoding uncompressed (v1) stereo WADY sound files works")
{
    do_test(
        "tests/fmt/ivory/files/wady/m01-zlib",
        "tests/fmt/ivory/files/wady/m01-zlib-out.wav");
}

TEST_CASE("Decoding compressed (v2) mono WADY sound files works")
{
    do_test(
        "tests/fmt/ivory/files/wady/10510-zlib",
        "tests/fmt/ivory/files/wady/10510-zlib-out.wav");
}

TEST_CASE("Decoding compressed (v2) stereo WADY sound files works")
{
    do_test(
        "tests/fmt/ivory/files/wady/071-zlib",
        "tests/fmt/ivory/files/wady/071-zlib-out.wav");
}
