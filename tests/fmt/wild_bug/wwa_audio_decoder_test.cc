#include "fmt/wild_bug/wwa_audio_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::wild_bug;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    WwaAudioDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_file = tests::file_from_path(expected_path);
    auto actual_file = decoder.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE("Wild Bug WWA audio using transcription strategy 3", "[fmt]")
{
    do_test(
        "tests/fmt/wild_bug/files/wwa/06A5A009.WWA",
        "tests/fmt/wild_bug/files/wwa/06A5A009-out.wav");
}
