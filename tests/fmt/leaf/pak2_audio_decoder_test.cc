#include "fmt/leaf/pak2_audio_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::leaf;

static const std::string dir = "tests/fmt/leaf/files/pak2-audio/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const Pak2AudioDecoder decoder;
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_file = tests::zlib_file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("Leaf PAK2 audio", "[fmt]")
{
    do_test("p1-zlib", "p1-out-zlib.wav");
}
