#include "fmt/lilim/dwv_audio_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::lilim;

TEST_CASE("Lilim DWV audio", "[fmt]")
{
    DwvAudioDecoder decoder;
    auto input_file = tests::file_from_path(
        "tests/fmt/lilim/files/dwv/SE010.dwv");
    auto expected_file = tests::file_from_path(
        "tests/fmt/lilim/files/dwv/SE010-out.wav");
    auto actual_file = tests::decode(decoder, *input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
