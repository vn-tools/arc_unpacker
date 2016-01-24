#include "dec/complets/vmd_audio_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::complets;

TEST_CASE("Complets VMD audio files", "[dec]")
{
    // Dumb test, but still a test
    const auto decoder = VmdAudioDecoder();
    io::File input_file(
        "test.vmd", "\x1A\x1E\x95\x96\x80\x90\x81\x8A\x88\x95\xD6"_b);
    const io::File expected_file("test.mp3", "\xFF\xFBpseudomp3"_b);
    const auto actual_file = tests::decode(decoder, input_file);
    tests::compare_files(*actual_file, expected_file, true);
}
