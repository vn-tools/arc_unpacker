#include "test_support/image_support.h"
#include <algorithm>
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

std::shared_ptr<util::Image> tests::image_from_file(File &file)
{
    file.io.seek(0);
    return util::Image::from_boxed(file.io);
}

std::shared_ptr<util::Image> tests::image_from_path(
    const boost::filesystem::path &path)
{
    return tests::image_from_file(*tests::file_from_path(path));
}

void tests::compare_images(
    const util::Image &expected_image, const util::Image &actual_image)
{
    REQUIRE(expected_image.pixels().width() == actual_image.pixels().width());
    REQUIRE(expected_image.pixels().height() == actual_image.pixels().height());

    for (auto y : util::range(expected_image.pixels().height()))
    for (auto x : util::range(expected_image.pixels().width()))
    {
        auto expected_pix = expected_image.pixels().at(x, y);
        auto actual_pix = actual_image.pixels().at(x, y);
        for (auto c : util::range(4))
            compare_pixels(expected_pix, actual_pix, x, y, c);
    }
}

void tests::compare_images(
    const std::vector<std::shared_ptr<util::Image>> &expected_images,
    const std::vector<std::shared_ptr<util::Image>> &actual_images)
{
    REQUIRE(expected_images.size() == actual_images.size());
    for (auto i : util::range(expected_images.size()))
        tests::compare_images(*expected_images[i], *actual_images[i]);
}

void tests::compare_images(
    const std::vector<std::shared_ptr<util::Image>> &expected_images,
    const std::vector<std::shared_ptr<File>> &actual_files)
{
    std::vector<std::shared_ptr<util::Image>> actual_images(
        actual_files.size());
    for (auto i : util::range(actual_files.size()))
    {
        actual_images[i]
            = util::Image::from_boxed(actual_files[i]->io);
    }
    tests::compare_images(expected_images, actual_images);
}
