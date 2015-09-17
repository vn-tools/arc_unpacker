#include "fmt/vorbis/packed_ogg_converter.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::vorbis;

TEST_CASE("Vorbis packed OGG audio", "[fmt]")
{
    PackedOggConverter converter;
    auto input_file = tests::file_from_path(
        "tests/fmt/vorbis/files/packed_ogg/1306.wav");
    auto expected_file = tests::file_from_path(
        "tests/fmt/vorbis/files/packed_ogg/1306-out.ogg");
    auto actual_file = converter.decode(*input_file);
    tests::compare_files(*expected_file, *actual_file, false);
}
