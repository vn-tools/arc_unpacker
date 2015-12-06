#include "res/pixel_format.h"
#include "test_support/catch.hh"
#include "util/format.h"
#include "util/range.h"

using namespace au;

static inline void compare_pixels(
    const res::Pixel expected, const res::Pixel actual)
{
    if (expected != actual)
    {
        FAIL(util::format(
            "Pixels differ: %02x%02x%02x%02x != %02x%02x%02x%02x",
            expected.b, expected.g, expected.r, expected.a,
            actual.b, actual.g, actual.r, actual.a));
    }
}

static void test_read(
    const u32 input_dword,
    const res::PixelFormat fmt,
    const res::Pixel expected_pixel)
{
    const auto bpp = res::pixel_format_to_bpp(fmt);
    bstr input_string(bpp);
    for (const auto i : util::range(bpp))
        input_string[i] = input_dword >> (i << 3);
    std::vector<res::Pixel> actual_pixels(1);
    res::read_pixels(input_string.get<u8>(), actual_pixels, fmt);
    const auto actual_pixel = actual_pixels[0];
    compare_pixels(expected_pixel, actual_pixel);
}

TEST_CASE("PixelFormat", "[res]")
{
    SECTION("Pixel format count")
    {
        REQUIRE(static_cast<int>(res::PixelFormat::Count) == 21);
    }

    SECTION("Getting BPP")
    {
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::Gray8) == 1);

        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::BGR555X) == 2);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::BGR565) == 2);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::BGRA4444) == 2);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::BGRA5551) == 2);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::BGRnA4444) == 2);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::BGRnA5551) == 2);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::RGB555X) == 2);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::RGB565) == 2);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::RGBA4444) == 2);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::RGBA5551) == 2);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::RGBnA4444) == 2);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::RGBnA5551) == 2);

        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::BGR888) == 3);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::RGB888) == 3);

        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::BGR888X) == 4);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::BGRA8888) == 4);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::BGRnA8888) == 4);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::RGB888X) == 4);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::RGBA8888) == 4);
        REQUIRE(res::pixel_format_to_bpp(res::PixelFormat::RGBnA8888) == 4);
    }

    SECTION("Reading")
    {
        using PF = res::PixelFormat;

        test_read(0b0000100011000111, PF::BGR565, {56, 24, 8, 0xFF});
        test_read(0b1000010001100111, PF::BGR555X, {56, 24, 8, 0xFF});
        test_read(0b0000010001100111, PF::BGR555X, {56, 24, 8, 0xFF});
        test_read(0b1000010001100111, PF::BGRA5551, {56, 24, 8, 0xFF});
        test_read(0b0000010001100111, PF::BGRA5551, {56, 24, 8, 0x00});
        test_read(0b1000010001100111, PF::BGRnA5551, {56, 24, 8, 0x00});
        test_read(0b0000010001100111, PF::BGRnA5551, {56, 24, 8, 0xFF});
        test_read(0b0001001101111111, PF::BGRA4444, {0xF0, 0x70, 0x30, 0x10});
        test_read(0b0001001101111111, PF::BGRnA4444, {0xF0, 0x70, 0x30, 0xEF});
        test_read(0b000000010000001000000011, PF::BGR888, {3, 2, 1, 0xFF});
        test_read(
            0b11111111000000010000001000000011, PF::BGR888X, {3, 2, 1, 0xFF});
        test_read(
            0b00000000000000010000001000000011, PF::BGR888X, {3, 2, 1, 0xFF});
        test_read(
            0b00000000000000010000001000000011, PF::BGRA8888, {3, 2, 1, 0});
        test_read(
            0b11111110000000010000001000000011, PF::BGRnA8888, {3, 2, 1, 1});

        test_read(0b0000100011000111, PF::RGB565, {8, 24, 56, 0xFF});
        test_read(0b1000010001100111, PF::RGB555X, {8, 24, 56, 0xFF});
        test_read(0b0000010001100111, PF::RGB555X, {8, 24, 56, 0xFF});
        test_read(0b1000010001100111, PF::RGBA5551, {8, 24, 56, 0xFF});
        test_read(0b0000010001100111, PF::RGBA5551, {8, 24, 56, 0x00});
        test_read(0b1000010001100111, PF::RGBnA5551, {8, 24, 56, 0x00});
        test_read(0b0000010001100111, PF::RGBnA5551, {8, 24, 56, 0xFF});
        test_read(0b0001001101111111, PF::RGBA4444, {0x30, 0x70, 0xF0, 0x10});
        test_read(0b0001001101111111, PF::RGBnA4444, {0x30, 0x70, 0xF0, 0xEF});
        test_read(0b000000010000001000000011, PF::RGB888, {1, 2, 3, 0xFF});
        test_read(
            0b11111111000000010000001000000011, PF::RGB888X, {1, 2, 3, 0xFF});
        test_read(
            0b00000000000000010000001000000011, PF::RGB888X, {1, 2, 3, 0xFF});
        test_read(
            0b00000000000000010000001000000011, PF::RGBA8888, {1, 2, 3, 0});
        test_read(
            0b11111110000000010000001000000011, PF::RGBnA8888, {1, 2, 3, 1});
    }
}
