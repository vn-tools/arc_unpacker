#include "fmt/amuse_craft/bgm_file_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"

using namespace au;
using namespace au::fmt::amuse_craft;

TEST_CASE("Amuse Craft BGM audio files", "[fmt]")
{
    // Dumb test, but still a test
    BgmFileDecoder decoder;
    File input_file("TEST.OGG", "BGM\x20JUNKJUNKOggSwhatever"_b);
    File expected_file("TEST.ogg", "OggSwhatever"_b);
    auto actual_file = tests::decode(decoder, input_file);
    tests::compare_files(expected_file, *actual_file, true);
}
