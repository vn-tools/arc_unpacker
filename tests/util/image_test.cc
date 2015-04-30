#include "io/file_io.h"
#include "test_support/catch.hpp"
#include "util/endian.h"
#include "util/image.h"

TEST_CASE("Reading transparent images works")
{
    FileIO io("tests/files/reimu_transparent.png", FileIOMode::Read);

    std::unique_ptr<Image> image = Image::from_boxed(io);
    REQUIRE(image->width() == 641);
    REQUIRE(image->height() == 720);

    uint32_t rgba = image->color_at(200, 100);
    REQUIRE(rgba == be32toh(0xfe0a17ff));
}

TEST_CASE("Reading opaque images works")
{
    FileIO io("tests/files/usagi_opaque.png", FileIOMode::Read);

    std::unique_ptr<Image> image = Image::from_boxed(io);
    REQUIRE(image->width() == 640);
    REQUIRE(image->height() == 480);

    uint32_t rgba = image->color_at(200, 100);
    REQUIRE(rgba == be32toh(0x7c6a34ff));
}
