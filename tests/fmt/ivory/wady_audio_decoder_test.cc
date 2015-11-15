#include "fmt/ivory/wady_audio_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::ivory;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const WadyAudioDecoder decoder;
    const auto input_file = tests::zlib_file_from_path(input_path);
    const auto expected_file = tests::zlib_file_from_path(expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("Ivory WADY uncompressed (v1) stereo audio", "[fmt]")
{
    do_test(
        "tests/fmt/ivory/files/wady/m01-zlib",
        "tests/fmt/ivory/files/wady/m01-zlib-out.wav");
}

TEST_CASE("Ivory WADY compressed (v2) mono audio", "[fmt]")
{
    do_test(
        "tests/fmt/ivory/files/wady/10510-zlib",
        "tests/fmt/ivory/files/wady/10510-zlib-out.wav");
}

TEST_CASE("Ivory WADY compressed (v2) stereo audio", "[fmt]")
{
    do_test(
        "tests/fmt/ivory/files/wady/071-zlib",
        "tests/fmt/ivory/files/wady/071-zlib-out.wav");
}
