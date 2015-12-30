#include "fmt/leaf/leafpack_group/p16_audio_decoder.h"
#include "test_support/audio_support.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::leaf;

static const std::string dir = "tests/fmt/leaf/files/p16/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = P16AudioDecoder();
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_file = tests::zlib_file_from_path(dir + expected_path);
    const auto actual_audio = tests::decode(decoder, *input_file);
    tests::compare_audio(*expected_file, actual_audio);
}

TEST_CASE("Leaf P16 audio", "[fmt]")
{
    do_test("PCM01-zlib.P16", "PCM01-zlib-out.wav");
}
