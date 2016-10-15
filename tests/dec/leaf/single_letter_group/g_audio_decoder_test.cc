#include "dec/leaf/single_letter_group/g_audio_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::dec::leaf;

static const std::string dir = "tests/dec/leaf/files/g/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = GAudioDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*actual_file, *expected_file, false);
}

TEST_CASE("Leaf G audio", "[dec]")
{
    do_test("asu_1400_080.g", "asu_1400_080-out.ogg");
}
