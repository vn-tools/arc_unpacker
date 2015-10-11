#include "fmt/twilight_frontier/tfwa_audio_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

TEST_CASE("Twilight Frontier TFWA audio", "[fmt]")
{
    TfwaAudioDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/twilight_frontier/files/tfwa/2592.wav");
    auto expected_file = tests::file_from_path(
        "tests/fmt/twilight_frontier/files/tfwa/2592-out.wav");
    auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
