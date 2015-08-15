#include "io/file_io.h"
#include "test_support/catch.hh"
#include "util/image.h"

using namespace au;
using namespace au::util;

TEST_CASE("Reading transparent PNG images works")
{
    io::FileIO io("tests/files/reimu_transparent.png", io::FileMode::Read);

    std::unique_ptr<Image> image = Image::from_boxed(io);
    REQUIRE(image->width() == 641);
    REQUIRE(image->height() == 720);

    auto color = image->color_at(200, 100);
    REQUIRE(static_cast<int>(color.r) == 0xFE);
    REQUIRE(static_cast<int>(color.g) == 0x0A);
    REQUIRE(static_cast<int>(color.b) == 0x17);
    REQUIRE(static_cast<int>(color.a) == 0xFF);
}

TEST_CASE("Reading opaque PNG images works")
{
    io::FileIO io("tests/files/usagi_opaque.png", io::FileMode::Read);

    std::unique_ptr<Image> image = Image::from_boxed(io);
    REQUIRE(image->width() == 640);
    REQUIRE(image->height() == 480);

    auto color = image->color_at(200, 100);
    REQUIRE(static_cast<int>(color.r) == 0x7C);
    REQUIRE(static_cast<int>(color.g) == 0x6A);
    REQUIRE(static_cast<int>(color.b) == 0x34);
    REQUIRE(static_cast<int>(color.a) == 0xFF);
}

TEST_CASE("Reading JPEG images works")
{
    io::FileIO io("tests/files/reimu_opaque.jpg", io::FileMode::Read);

    std::unique_ptr<Image> image = Image::from_boxed(io);
    REQUIRE(image->width() == 1024);
    REQUIRE(image->height() == 1024);

    auto color = image->color_at(200, 100);
    REQUIRE(static_cast<int>(color.r) == 0x60);
    REQUIRE(static_cast<int>(color.g) == 0x97);
    REQUIRE(static_cast<int>(color.b) == 0xE7);
    REQUIRE(static_cast<int>(color.a) == 0xFF);
}
