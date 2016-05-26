#include "dec/triangle/wady_audio_decoder.h"
#include "test_support/audio_support.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::triangle;

static const std::string dir = "tests/dec/triangle/files/wady/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = WadyAudioDecoder();
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_file = tests::zlib_file_from_path(dir + expected_path);
    const auto actual_audio = tests::decode(decoder, *input_file);
    tests::compare_audio(actual_audio, *expected_file);
}

TEST_CASE("Triangle WADY audio", "[dec]")
{
    do_test(
        "M03-zlib",
        "M03-out-zlib.wav");
}
