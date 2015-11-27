#include "test_support/image_support.h"
#include "fmt/jpeg/jpeg_image_decoder.h"
#include "fmt/microsoft/bmp_image_decoder.h"
#include "fmt/png/png_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;

static inline void compare_pixels(
    const pix::Pixel expected,
    const pix::Pixel actual,
    const size_t x,
    const size_t y,
    const size_t c)
{
    if (expected[c] == actual[c])
        return;

    INFO(util::format(
        "Pixels differ at %d, %d: %02x%02x%02x%02x != %02x%02x%02x%02x",
        x, y,
        expected.b, expected.g, expected.r, expected.a,
        actual.b, actual.g, actual.r, actual.a));
    REQUIRE(expected.r == actual.r);
    REQUIRE(expected.g == actual.g);
    REQUIRE(expected.b == actual.b);
    REQUIRE(expected.a == actual.a);
}

static pix::Image image_from_file(io::File &file)
{
    static const fmt::png::PngImageDecoder png_image_decoder;
    if (png_image_decoder.is_recognized(file))
        return png_image_decoder.decode(file);

    static const fmt::microsoft::BmpImageDecoder bmp_image_decoder;
    if (bmp_image_decoder.is_recognized(file))
        return bmp_image_decoder.decode(file);

    static const fmt::jpeg::JpegImageDecoder jpeg_image_decoder;
    if (jpeg_image_decoder.is_recognized(file))
        return jpeg_image_decoder.decode(file);

    throw std::logic_error("Only PNG, BMP and JPEG files are supported");
}

void tests::compare_images(
    const pix::Image &expected_image, const pix::Image &actual_image)
{
    REQUIRE(expected_image.width() == actual_image.width());
    REQUIRE(expected_image.height() == actual_image.height());

    for (const auto y : util::range(expected_image.height()))
    for (const auto x : util::range(expected_image.width()))
    {
        const auto expected_pix = expected_image.at(x, y);
        const auto actual_pix = actual_image.at(x, y);
        for (const auto c : util::range(4))
            compare_pixels(expected_pix, actual_pix, x, y, c);
    }
}

void tests::compare_images(
    io::File &expected_file, const pix::Image &actual_image)
{
    tests::compare_images(image_from_file(expected_file), actual_image);
}

void tests::compare_images(
    io::File &expected_file,
    io::File &actual_file,
    const bool compare_file_names)
{
    auto expected_image = image_from_file(expected_file);
    auto actual_image = image_from_file(actual_file);
    if (compare_file_names)
        tests::compare_file_names(expected_file.name, actual_file.name);
    tests::compare_images(expected_image, actual_image);
}

void tests::compare_images(
    const std::vector<std::shared_ptr<io::File>> &expected_images,
    const std::vector<std::shared_ptr<io::File>> &actual_images,
    const bool compare_file_names)
{
    REQUIRE(expected_images.size() == actual_images.size());
    for (const auto i : util::range(expected_images.size()))
    {
        INFO(util::format("Images at index %d differ", i));
        tests::compare_images(
            *expected_images[i], *actual_images[i], compare_file_names);
    }
}
