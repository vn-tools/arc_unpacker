#include "fmt/twilight_frontier/pak2_audio_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

TEST_CASE("Twilight Frontier CV3 audio", "[fmt]")
{
    const Pak2AudioDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/twilight_frontier/files/pak2/049.cv3");
    const auto expected_file = tests::file_from_path(
        "tests/fmt/twilight_frontier/files/pak2/049-out.wav");
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
