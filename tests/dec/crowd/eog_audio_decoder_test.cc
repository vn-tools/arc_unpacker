#include "dec/crowd/eog_audio_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::crowd;

TEST_CASE("Crowd EOG audio files", "[dec]")
{
    // Dumb test, but still a test
    const auto decoder = EogAudioDecoder();
    io::File input_file("TEST.OGG", "CRM\x00\x0C\x00\x00\x00OggSwhatever"_b);
    const io::File expected_file("TEST.ogg", "OggSwhatever"_b);
    const auto actual_file = tests::decode(decoder, input_file);
    tests::compare_files(*actual_file, expected_file, true);
}
