#include <algorithm>
#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;

static inline void compare_pixels(
    pix::Pixel expected,
    pix::Pixel actual,
    size_t x,
    size_t y,
    size_t c,
    size_t max_component_diff)
{
    if (std::abs(expected[c] - actual[c]) > max_component_diff)
    {
        INFO(util::format(
            "Pixels differ at %d, %d: %02x%02x%02x%02x != %02x%02x%02x%02x",
            x, y,
            expected.b, expected.g, expected.r, expected.a,
            actual.b, actual.g, actual.r, actual.a));
        REQUIRE(std::abs(expected.r - actual.r) <= max_component_diff);
        REQUIRE(std::abs(expected.g - actual.g) <= max_component_diff);
        REQUIRE(std::abs(expected.b - actual.b) <= max_component_diff);
        REQUIRE(std::abs(expected.a - actual.a) <= max_component_diff);
    }
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
    const util::Image &expected_image,
    const util::Image &actual_image,
    int max_component_diff)
{
    REQUIRE(expected_image.pixels().width() == actual_image.pixels().width());
    REQUIRE(expected_image.pixels().height() == actual_image.pixels().height());

    for (auto y : util::range(expected_image.pixels().height()))
    for (auto x : util::range(expected_image.pixels().width()))
    {
        auto expected_pix = expected_image.pixels().at(x, y);
        auto actual_pix = actual_image.pixels().at(x, y);
        for (auto c : util::range(4))
        {
            compare_pixels(
                expected_pix, actual_pix, x, y, c, max_component_diff);
        }
    }
}
