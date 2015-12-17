#include "fmt/ivory/wady_audio_decoder.h"
#include "test_support/audio_support.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::ivory;

static const std::string dir = "tests/fmt/ivory/files/wady/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const WadyAudioDecoder decoder;
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_file = tests::zlib_file_from_path(dir + expected_path);
    const auto actual_audio = tests::decode(decoder, *input_file);
    tests::compare_audio(*expected_file, actual_audio);
}

TEST_CASE("Ivory WADY audio", "[fmt]")
{
    SECTION("Version 1 (uncompressed), stereo")
    {
        do_test("m01-zlib", "m01-zlib-out.wav");
    }

    SECTION("Version 2 (compressed), mono")
    {
        do_test("10510-zlib", "10510-zlib-out.wav");
    }

    SECTION("Version 2 (compressed), stereo")
    {
        do_test("071-zlib", "071-zlib-out.wav");
    }
}
