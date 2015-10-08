#include "fmt/png/png_image_decoder.h"
#include "io/file_io.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"

using namespace au;
using namespace au::fmt::png;

TEST_CASE("PNG images with transparency", "[fmt]")
{
    File file("tests/fmt/png/files/reimu_transparent.png", io::FileMode::Read);

    PngImageDecoder decoder;
    auto pixels = tests::decode(decoder, file);
    REQUIRE(pixels.width() == 641);
    REQUIRE(pixels.height() == 720);

    auto color = pixels.at(200, 100);
    REQUIRE(static_cast<int>(color.r) == 0xFE);
    REQUIRE(static_cast<int>(color.g) == 0x0A);
    REQUIRE(static_cast<int>(color.b) == 0x17);
    REQUIRE(static_cast<int>(color.a) == 0xFF);
}

TEST_CASE("Reading opaque PNG images", "[util]")
{
    File file("tests/fmt/png/files/usagi_opaque.png", io::FileMode::Read);

    PngImageDecoder decoder;
    auto pixels = tests::decode(decoder, file);
    REQUIRE(pixels.width() == 640);
    REQUIRE(pixels.height() == 480);

    auto color = pixels.at(200, 100);
    REQUIRE(static_cast<int>(color.r) == 0x7C);
    REQUIRE(static_cast<int>(color.g) == 0x6A);
    REQUIRE(static_cast<int>(color.b) == 0x34);
    REQUIRE(static_cast<int>(color.a) == 0xFF);
}
