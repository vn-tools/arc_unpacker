#include "fmt/entis/mio_audio_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::entis;

static const std::string dir = "tests/fmt/entis/files/mio/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const MioAudioDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("Entis MIO lossy audio", "[fmt]")
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
