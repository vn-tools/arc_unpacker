#include "fmt/leaf/pak1_group/grp_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

static const std::string pak1_dir = "tests/fmt/leaf/files/pak1/";
static const std::string grp_dir = "tests/fmt/leaf/files/grp/";

TEST_CASE("Leaf GRP images", "[fmt]")
{
    const GrpImageDecoder decoder;

    SECTION("Palettes")
    {
        const auto palette_file
            = tests::file_from_path(pak1_dir + "leaflogo-out.c16");
        const auto input_file
            = tests::file_from_path(pak1_dir + "leaflogo-out.grp");
        const auto expected_file
            = tests::image_from_path(grp_dir + "leaflogo-out.png");

        const auto actual_file
            = decoder.decode(*input_file, palette_file, nullptr);
        tests::compare_images(*expected_file, actual_file);
    }

    SECTION("Palettes, variant with extra 0 bytes at beginning")
    {
        const auto palette_file
            = tests::file_from_path(pak1_dir + "leaf-out.c16");
        const auto input_file
            = tests::file_from_path(pak1_dir + "leaf-out.grp");
        const auto expected_file
            = tests::image_from_path(grp_dir + "leaf-out.png");

        const auto actual_file
            = decoder.decode(*input_file, palette_file, nullptr);
        tests::compare_images(*expected_file, actual_file);
    }

    SECTION("Palettes and masks")
    {
        const auto palette_file = tests::file_from_path(grp_dir + "ase200.c16");
        const auto mask_file = tests::file_from_path(grp_dir + "ase200.msk");
        const auto input_file = tests::file_from_path(grp_dir + "ase200.grp");
        const auto expected_file
            = tests::image_from_path(grp_dir + "ase200-out.png");

        const auto actual_file
            = decoder.decode(*input_file, palette_file, mask_file);
        tests::compare_images(*expected_file, actual_file);
    }
}
