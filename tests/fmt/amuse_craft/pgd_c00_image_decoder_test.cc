#include "fmt/amuse_craft/pgd_c00_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::amuse_craft;

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const PgdC00ImageDecoder decoder;
    const auto input_file = tests::zlib_file_from_path(input_path);
    const auto expected_image = tests::image_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Amuse Craft PGD images (C00 / TGA)", "[fmt]")
{
    do_test(
        "tests/fmt/amuse_craft/files/pgd-c00/FA03C_01E-zlib.PGD",
        "tests/fmt/amuse_craft/files/pgd-c00/FA03C_01E-out.png");
}
