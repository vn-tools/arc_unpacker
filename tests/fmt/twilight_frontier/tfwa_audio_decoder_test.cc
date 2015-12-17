#include "fmt/twilight_frontier/tfwa_audio_decoder.h"
#include "test_support/audio_support.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

static const std::string dir = "tests/fmt/twilight_frontier/files/tfwa/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const TfwaAudioDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_audio = tests::decode(decoder, *input_file);
    tests::compare_audio(*expected_file, actual_audio);
}

TEST_CASE("Twilight Frontier TFWA audio", "[fmt]")
{
    do_test("2592.wav", "2592-out.wav");
}
