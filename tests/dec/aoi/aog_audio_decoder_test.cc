#include "dec/aoi/aog_audio_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::aoi;

TEST_CASE("Aoi AOG audio files", "[dec]")
{
    const auto decoder = AogAudioDecoder();
    io::File input_file(
        "test.aog",
        "AoiOgg\x00\x00JUNKJUNKJUNKJUNKJUNKJUNKJUNKJUNKJUNKOggSwhatever"_b);
    const io::File expected_file("test.ogg", "OggSwhatever"_b);
    const auto actual_file = tests::decode(decoder, input_file);
    tests::compare_files(*actual_file, expected_file, true);
}
