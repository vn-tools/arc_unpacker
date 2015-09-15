#include "fmt/liar_soft/packed_ogg_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::liar_soft;

TEST_CASE("Decoding LiarSoft's packed OGG sound files works", "[fmt]")
{
    PackedOggConverter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/liar_soft/files/packed_ogg/1306.wav");
    auto expected_file = tests::file_from_path(
        "tests/fmt/liar_soft/files/packed_ogg/1306-out.ogg");
    auto actual_file = converter.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
