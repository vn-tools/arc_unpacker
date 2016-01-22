#include "dec/real_live/nwa_audio_decoder.h"
#include "test_support/audio_support.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::real_live;

static const std::string dir = "tests/dec/real_live/files/nwa/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = NwaAudioDecoder();
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_file = tests::zlib_file_from_path(dir + expected_path);
    const auto actual_audio = tests::decode(decoder, *input_file);
    tests::compare_audio(actual_audio, *expected_file);
}

TEST_CASE("RealLive NWA audio", "[dec]")
{
    SECTION("Level 0-compressed, mono")
    {
        do_test("BT_KOE_HCC01-zlib.nwa", "BT_KOE_HCC01-zlib-out.wav");
    }

    SECTION("Level 0-compressed, stereo")
    {
        do_test("BATSWING-zlib.nwa", "BATSWING-zlib-out.wav");
    }
}
