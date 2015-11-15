#include "fmt/vorbis/packed_ogg_audio_decoder.h"
#include "log.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::vorbis;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const PackedOggAudioDecoder decoder;
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_file = tests::file_from_path(expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("Vorbis packed OGG audio", "[fmt]")
{
    do_test(
        "tests/fmt/vorbis/files/packed_ogg/1306.wav",
        "tests/fmt/vorbis/files/packed_ogg/1306-out.ogg");
}

TEST_CASE("Vorbis packed OGG audio with early EOF", "[fmt]")
{
    Log.mute();
    do_test(
        "tests/fmt/vorbis/files/packed_ogg/90WIF020_001.WAV",
        "tests/fmt/vorbis/files/packed_ogg/90WIF020_001-out.ogg");
}
