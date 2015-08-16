#include "fmt/touhou/pak2_sound_converter.h"
#include "test_support/catch.hh"
#include "test_support/converter_support.h"

using namespace au;
using namespace au::fmt::touhou;

TEST_CASE("Decoding CV3 sound files works")
{
    Pak2SoundConverter converter;
    tests::assert_file_conversion(
        converter,
        "tests/fmt/touhou/files/pak2/049.cv3",
        "tests/fmt/touhou/files/pak2/049-out.wav");
}
