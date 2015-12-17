#include "fmt/leaf/single_letter_group/w_audio_decoder.h"
#include "test_support/audio_support.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::leaf;

static const std::string dir = "tests/fmt/leaf/files/w/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const WAudioDecoder decoder;
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_file = tests::zlib_file_from_path(dir + expected_path);
    const auto actual_audio = tests::decode(decoder, *input_file);
    tests::compare_audio(*expected_file, actual_audio);
}

TEST_CASE("Leaf W audio", "[fmt]")
{
    do_test("se068-zlib.w", "se068-zlib-out.wav");
}
