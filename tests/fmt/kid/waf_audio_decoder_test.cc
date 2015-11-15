#include "fmt/kid/waf_audio_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::kid;

static const std::string dir = "tests/fmt/kid/files/waf/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const WafAudioDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("KID WAF audio", "[fmt]")
{
    do_test(
        "CEP037.waf",
        "CEP037-out.wav");
}
