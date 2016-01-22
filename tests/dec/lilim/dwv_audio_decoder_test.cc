#include "dec/lilim/dwv_audio_decoder.h"
#include "test_support/audio_support.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::lilim;

static const std::string dir = "tests/dec/lilim/files/dwv/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = DwvAudioDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_audio = tests::decode(decoder, *input_file);
    tests::compare_audio(actual_audio, *expected_file);
}

TEST_CASE("Lilim DWV audio", "[dec]")
{
    do_test("SE010.dwv", "SE010-out.wav");
}
