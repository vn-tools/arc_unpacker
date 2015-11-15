#include "fmt/kid/waf_audio_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::kid;

TEST_CASE("KID WAF audio", "[fmt]")
{
    const WafAudioDecoder decoder;
    const auto input_file = tests::file_from_path(
        "tests/fmt/kid/files/waf/CEP037.waf");
    const auto expected_file = tests::file_from_path(
        "tests/fmt/kid/files/waf/CEP037-out.wav");
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
