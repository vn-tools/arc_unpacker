#include "fmt/leaf/grp_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

TEST_CASE("Leaf GRP images", "[fmt]")
{
    const auto palette_file
        = tests::file_from_path("tests/fmt/leaf/files/pak1/leaflogo-out.c16");
    const auto input_file
        = tests::file_from_path("tests/fmt/leaf/files/pak1/leaflogo-out.grp");
    const auto expected_file
        = tests::image_from_path("tests/fmt/leaf/files/grp/leaflogo-out.png");

    const GrpImageDecoder decoder;
    const auto actual_file = decoder.decode(*input_file, palette_file, nullptr);
    tests::compare_images(*expected_file, actual_file);
}

TEST_CASE("Leaf GRP images (variant with extra 0 bytes at beginning)", "[fmt]")
{
    const auto palette_file
        = tests::file_from_path("tests/fmt/leaf/files/pak1/leaf-out.c16");
    const auto input_file
        = tests::file_from_path("tests/fmt/leaf/files/pak1/leaf-out.grp");
    const auto expected_file
        = tests::image_from_path("tests/fmt/leaf/files/grp/leaf-out.png");

    const GrpImageDecoder decoder;
    const auto actual_file = decoder.decode(*input_file, palette_file, nullptr);
    tests::compare_images(*expected_file, actual_file);
}

TEST_CASE("Leaf GRP images (palettes and masks)", "[fmt]")
{
    const auto palette_file
        = tests::file_from_path("tests/fmt/leaf/files/grp/ase200.c16");
    const auto mask_file
        = tests::file_from_path("tests/fmt/leaf/files/grp/ase200.msk");
    const auto input_file
        = tests::file_from_path("tests/fmt/leaf/files/grp/ase200.grp");
    const auto expected_file
        = tests::image_from_path("tests/fmt/leaf/files/grp/ase200-out.png");

    const GrpImageDecoder decoder;
    const auto actual_file
        = decoder.decode(*input_file, palette_file, mask_file);
    tests::compare_images(*expected_file, actual_file);
}
