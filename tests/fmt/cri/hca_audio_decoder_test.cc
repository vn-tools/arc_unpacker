#include "fmt/cri/hca_audio_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::cri;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    HcaAudioDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_file = tests::file_from_path(expected_path);
    auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}

TEST_CASE(
    "CRI HCA audio (mono, unlooped, cipher 0, no 'dec' chunk, "
        "no advanced compression)",
    "[fmt]")
{
    do_test(
        "tests/fmt/cri/files/hca/test.hca",
        "tests/fmt/cri/files/hca/test-out.wav");
}
