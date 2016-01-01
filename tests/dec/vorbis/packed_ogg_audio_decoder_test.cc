#include "dec/vorbis/packed_ogg_audio_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::vorbis;

static const std::string dir = "tests/dec/vorbis/files/packed_ogg/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = PackedOggAudioDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("Vorbis packed OGG audio", "[dec]")
{
    SECTION("Plain")
    {
        do_test("1306.wav", "1306-out.ogg");
    }

    SECTION("Early EOF")
    {
        do_test("90WIF020_001.WAV", "90WIF020_001-out.ogg");
    }
}
