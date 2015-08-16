#include "test_support/catch.hh"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;

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
        if (expected_pix != actual_pix) //speed up
        {
            INFO(util::format(
                "Pixels differ at %d, %d: %02x%02x%02x%02x != %02x%02x%02x%02x",
                x, y,
                expected_pix.b, expected_pix.g, expected_pix.r, expected_pix.a,
                actual_pix.b, actual_pix.g, actual_pix.r, actual_pix.a));
            REQUIRE(expected_pix == actual_pix);
        }
    }
}

std::shared_ptr<util::Image> tests::image_from_path(
    const boost::filesystem::path &path)
{
    return util::Image::from_boxed(tests::file_from_path(path)->io);
}
