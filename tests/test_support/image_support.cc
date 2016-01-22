#include "test_support/image_support.h"
#include "algo/format.h"
#include "algo/range.h"
#include "dec/jpeg/jpeg_image_decoder.h"
#include "dec/microsoft/bmp_image_decoder.h"
#include "dec/png/png_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/file_support.h"

using namespace au;

static inline void compare_pixels(
    const res::Pixel actual,
    const res::Pixel expected,
    const size_t x,
    const size_t y)
{
    if (expected != actual)
    {
        if (expected.a == 0 && actual.a == 0)
            return;
        FAIL(algo::format(
            "Pixels differ at %d, %d: %02x%02x%02x%02x != %02x%02x%02x%02x",
            x, y,
            actual.b, actual.g, actual.r, actual.a,
            expected.b, expected.g, expected.r, expected.a));
    }
}

static res::Image image_from_file(io::File &file)
{
    Logger dummy_logger;
    dummy_logger.mute();

    static const auto png_image_decoder = dec::png::PngImageDecoder();
    if (png_image_decoder.is_recognized(file))
        return png_image_decoder.decode(dummy_logger, file);

    static const auto bmp_image_decoder = dec::microsoft::BmpImageDecoder();
    if (bmp_image_decoder.is_recognized(file))
        return bmp_image_decoder.decode(dummy_logger, file);

    static const auto jpeg_image_decoder = dec::jpeg::JpegImageDecoder();
    if (jpeg_image_decoder.is_recognized(file))
        return jpeg_image_decoder.decode(dummy_logger, file);

    throw std::logic_error("Only PNG, BMP and JPEG files are supported");
}

res::Image tests::get_opaque_test_image()
{
    const auto input_file = tests::file_from_path("tests/dec/homura.png");
    return image_from_file(*input_file);
}

void tests::compare_images(
    const res::Image &actual_image, const res::Image &expected_image)
{
    REQUIRE(expected_image.width() == actual_image.width());
    REQUIRE(expected_image.height() == actual_image.height());

    for (const auto y : algo::range(expected_image.height()))
    for (const auto x : algo::range(expected_image.width()))
    {
        const auto expected_pixel = expected_image.at(x, y);
        const auto actual_pixel = actual_image.at(x, y);
        compare_pixels(expected_pixel, actual_pixel, x, y);
    }
}

void tests::compare_images(
    const res::Image &actual_image, io::File &expected_file)
{
    tests::compare_images(actual_image, image_from_file(expected_file));
}

void tests::compare_images(
    io::File &actual_file,
    io::File &expected_file,
    const bool compare_file_paths)
{
    auto expected_image = image_from_file(expected_file);
    auto actual_image = image_from_file(actual_file);
    if (compare_file_paths)
        tests::compare_paths(actual_file.path, expected_file.path);
    tests::compare_images(actual_image, expected_image);
}

void tests::compare_images(
    const std::vector<std::shared_ptr<io::File>> &actual_images,
    const std::vector<std::shared_ptr<io::File>> &expected_images,
    const bool compare_file_paths)
{
    REQUIRE(expected_images.size() == actual_images.size());
    for (const auto i : algo::range(expected_images.size()))
    {
        INFO(algo::format("Images at index %d differ", i));
        tests::compare_images(
            *actual_images[i], *expected_images[i], compare_file_paths);
    }
}
