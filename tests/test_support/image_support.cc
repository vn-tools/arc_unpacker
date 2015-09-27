#include "test_support/image_support.h"
#include <algorithm>
#include "fmt/jpeg/jpeg_image_decoder.h"
#include "fmt/png/png_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;

static inline void compare_pixels(
    pix::Pixel expected, pix::Pixel actual, size_t x, size_t y, size_t c)
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

std::shared_ptr<pix::Grid> tests::image_from_file(File &file)
{
    fmt::jpeg::JpegImageDecoder jpeg_image_decoder;
    fmt::png::PngImageDecoder png_image_decoder;
    if (jpeg_image_decoder.is_recognized(file))
        return std::make_shared<pix::Grid>(jpeg_image_decoder.decode(file));
    if (png_image_decoder.is_recognized(file))
        return std::make_shared<pix::Grid>(png_image_decoder.decode(file));
    throw std::logic_error("Only JPEG and PNG files are supported");
}

std::shared_ptr<pix::Grid> tests::image_from_path(
    const boost::filesystem::path &path)
{
    return tests::image_from_file(*tests::file_from_path(path));
}

void tests::compare_images(
    const pix::Grid &expected_image, const pix::Grid &actual_image)
{
    REQUIRE(expected_image.width() == actual_image.width());
    REQUIRE(expected_image.height() == actual_image.height());

    for (auto y : util::range(expected_image.height()))
    for (auto x : util::range(expected_image.width()))
    {
        auto expected_pix = expected_image.at(x, y);
        auto actual_pix = actual_image.at(x, y);
        for (auto c : util::range(4))
            compare_pixels(expected_pix, actual_pix, x, y, c);
    }
}

void tests::compare_images(
    const std::vector<std::shared_ptr<pix::Grid>> &expected_images,
    const std::vector<std::shared_ptr<pix::Grid>> &actual_images)
{
    REQUIRE(expected_images.size() == actual_images.size());
    for (auto i : util::range(expected_images.size()))
    {
        INFO(util::format("Images at index %d differ", i));
        tests::compare_images(*expected_images[i], *actual_images[i]);
    }
}

void tests::compare_images(
    const std::vector<std::shared_ptr<pix::Grid>> &expected_images,
    const std::vector<std::shared_ptr<File>> &actual_files)
{
    std::vector<std::shared_ptr<pix::Grid>> actual_images(
        actual_files.size());
    for (auto i : util::range(actual_files.size()))
        actual_images[i] = tests::image_from_file(*actual_files[i]);
    tests::compare_images(expected_images, actual_images);
}
