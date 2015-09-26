#include "fmt/touhou/tfwa_audio_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::touhou;

TEST_CASE("Touhou TFWA audio", "[fmt]")
{
    TfwaAudioDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/touhou/files/tfwa/2592.wav");
    auto expected_file = tests::file_from_path(
        "tests/fmt/touhou/files/tfwa/2592-out.wav");
    auto actual_file = decoder.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
