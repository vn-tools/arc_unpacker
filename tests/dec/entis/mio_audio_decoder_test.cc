#include "dec/entis/mio_audio_decoder.h"
#include "test_support/audio_support.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::entis;

static const std::string dir = "tests/dec/entis/files/mio/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = MioAudioDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_audio = tests::decode(decoder, *input_file);
    tests::compare_audio(*expected_file, actual_audio);
}

TEST_CASE("Entis MIO lossy audio", "[dec]")
{
    SECTION("LOT/DCT")
    {
        do_test("explosion.mio", "explosion-out.wav");
    }

    SECTION("LOT/DCT+MSS")
    {
        do_test("SE_017.mio", "SE_017-out.wav");
    }
}
