#include "fmt/jpeg/jpeg_image_decoder.h"
#include "io/file_stream.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::jpeg;

static const std::string dir = "tests/fmt/jpeg/files/";

TEST_CASE("JPEG 24-bit images", "[util]")
{
    const auto input_file = tests::file_from_path(dir + "reimu_opaque.jpg");

    const JpegImageDecoder decoder;
    const auto pixels = tests::decode(decoder, *input_file);
    REQUIRE(pixels.width() == 1024);
    REQUIRE(pixels.height() == 1024);

    const auto color = pixels.at(200, 100);
    REQUIRE(static_cast<int>(color.r) == 0x60);
    REQUIRE(static_cast<int>(color.g) == 0x97);
    REQUIRE(static_cast<int>(color.b) == 0xE7);
    REQUIRE(static_cast<int>(color.a) == 0xFF);
}

TEST_CASE("JPEG 8-bit images", "[util]")
{
    JpegImageDecoder decoder;
    auto input_file = tests::file_from_path(dir + "NoName.jpeg");
    auto expected_image = tests::image_from_path(dir + "NoName-out.png");
    auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}
