#include "io/file_io.h"
#include "test_support/catch.hpp"
#include "util/endian.h"
#include "util/image.h"

using namespace au;
using namespace au::util;

TEST_CASE("Reading transparent images works")
{
    io::FileIO io("tests/files/reimu_transparent.png", io::FileMode::Read);

    std::unique_ptr<Image> image = Image::from_boxed(io);
    REQUIRE(image->width() == 641);
    REQUIRE(image->height() == 720);

    u32 rgba = image->color_at(200, 100);
    REQUIRE(rgba == util::from_big_endian<u32>(0xFE0A17FF));
}

TEST_CASE("Reading opaque images works")
{
    io::FileIO io("tests/files/usagi_opaque.png", io::FileMode::Read);

    std::unique_ptr<Image> image = Image::from_boxed(io);
    REQUIRE(image->width() == 640);
    REQUIRE(image->height() == 480);

    u32 rgba = image->color_at(200, 100);
    REQUIRE(rgba == util::from_big_endian<u32>(0x7C6A34FF));
}
