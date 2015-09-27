#include "fmt/jpeg/jpeg_image_decoder.h"
#include "io/file_io.h"
#include "test_support/catch.hh"

using namespace au;
using namespace au::fmt::jpeg;

TEST_CASE("Reading JPEG images", "[util]")
{
    File file("tests/fmt/jpeg/files/reimu_opaque.jpg", io::FileMode::Read);

    JpegImageDecoder jpeg_image_decoder;
    auto pixels = jpeg_image_decoder.decode(file);
    REQUIRE(pixels.width() == 1024);
    REQUIRE(pixels.height() == 1024);

    auto color = pixels.at(200, 100);
    REQUIRE(static_cast<int>(color.r) == 0x60);
    REQUIRE(static_cast<int>(color.g) == 0x97);
    REQUIRE(static_cast<int>(color.b) == 0xE7);
    REQUIRE(static_cast<int>(color.a) == 0xFF);
}
