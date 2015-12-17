#include "fmt/bgi/bgi_audio_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::bgi;

static const std::string dir = "tests/fmt/bgi/files/audio/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const BgiAudioDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("BGI audio", "[fmt]")
{
    do_test("wa_038.dat", "wa_038-out.ogg");
}
