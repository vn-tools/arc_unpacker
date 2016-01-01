#include "dec/amuse_craft/bgm_audio_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::amuse_craft;

TEST_CASE("Amuse Craft BGM audio files", "[dec]")
{
    // Dumb test, but still a test
    const auto decoder = BgmAudioDecoder();
    io::File input_file("TEST.OGG", "BGM\x20JUNKJUNKOggSwhatever"_b);
    const io::File expected_file("TEST.ogg", "OggSwhatever"_b);
    const auto actual_file = tests::decode(decoder, input_file);
    tests::compare_files(expected_file, *actual_file, true);
}
