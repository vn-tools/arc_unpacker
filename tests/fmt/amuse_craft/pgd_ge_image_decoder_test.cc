#include "fmt/amuse_craft/pgd_ge_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::amuse_craft;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    PgdGeImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto expected_image = tests::image_from_path(expected_path);
    auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Amuse Craft PGD images (GE / 24-bit / filter 2)", "[fmt]")
{
    do_test(
        "tests/fmt/amuse_craft/files/pgd-ge/ETLA023R.PGD",
        "tests/fmt/amuse_craft/files/pgd-ge/ETLA023R-out.png");
}

TEST_CASE("Amuse Craft PGD images (GE / 24-bit / filter 3)", "[fmt]")
{
    do_test(
        "tests/fmt/amuse_craft/files/pgd-ge/SECRET.PGD",
        "tests/fmt/amuse_craft/files/pgd-ge/SECRET-out.png");
}

TEST_CASE("Amuse Craft PGD images (GE / 32-bit / filter 3)", "[fmt]")
{
    do_test(
        "tests/fmt/amuse_craft/files/pgd-ge/SYSTEM.PGD",
        "tests/fmt/amuse_craft/files/pgd-ge/SYSTEM-out.png");
}
