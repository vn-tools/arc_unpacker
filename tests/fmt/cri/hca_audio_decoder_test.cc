#include "fmt/cri/hca_audio_decoder.h"
#include "test_support/audio_support.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::cri;

static const std::string dir = "tests/fmt/cri/files/hca/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const HcaAudioDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_audio(*expected_file, *actual_file, false);
}

TEST_CASE("CRI HCA audio", "[fmt]")
{
    SECTION("Mono, unlooped, cipher 0, no 'dec' chunk, no advanced compression")
    {
        do_test("test.hca", "test-out.wav");
    }
}
