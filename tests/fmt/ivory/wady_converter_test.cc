#include "fmt/ivory/wady_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::ivory;

TEST_CASE("Decoding uncompressed (v1) WADY sound files works")
{
    auto input = tests::zlib_file_from_path(
        "tests/fmt/ivory/files/wady/m01-zlib");
    auto expected = tests::zlib_file_from_path(
        "tests/fmt/ivory/files/wady/m01-zlib-out.wav");

    WadyConverter converter;
    auto actual = converter.decode(*input);
    tests::compare_files(*expected, *actual, false);
}

TEST_CASE("Decoding compressed (v2) WADY sound files works")
{
    auto input = tests::zlib_file_from_path(
        "tests/fmt/ivory/files/wady/071-zlib");
    auto expected = tests::zlib_file_from_path(
        "tests/fmt/ivory/files/wady/071-zlib-out.wav");

    WadyConverter converter;
    auto actual = converter.decode(*input);
    tests::compare_files(*expected, *actual, false);
}
