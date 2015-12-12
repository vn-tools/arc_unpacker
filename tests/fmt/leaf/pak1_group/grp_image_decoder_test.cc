#include "fmt/leaf/pak1_group/grp_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "util/file_from_image.h"

using namespace au;
using namespace au::fmt::leaf;

static const std::string pak1_dir = "tests/fmt/leaf/files/pak1/";
static const std::string grp_dir = "tests/fmt/leaf/files/grp/";

static void do_test(
    const std::string &input_path,
    const std::string &palette_path,
    const std::string &mask_path,
    const std::string &expected_path)
{
    const GrpImageDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto input_palette_file = palette_path.empty()
        ? nullptr
        : tests::file_from_path(palette_path);
    auto input_mask_file = mask_path.empty()
        ? nullptr
        : tests::file_from_path(mask_path);
    auto expected_file = tests::file_from_path(expected_path);

    const auto actual_image = decoder.decode(
        *input_file, std::move(input_palette_file), std::move(input_mask_file));
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Leaf GRP images", "[fmt]")
{
    SECTION("Palettes")
    {
        do_test(
            pak1_dir + "leaflogo-out.grp",
            pak1_dir + "leaflogo-out.c16",
            "",
            grp_dir + "leaflogo-out.png");
}

    SECTION("Palettes, variant with extra 0 bytes at beginning")
    {
        do_test(
            pak1_dir + "leaf-out.grp",
            pak1_dir + "leaf-out.c16",
            "",
            grp_dir + "leaf-out.png");
    }

    SECTION("Palettes and masks")
    {
        do_test(
            grp_dir + "ase200.grp",
            grp_dir + "ase200.c16",
            grp_dir + "ase200.msk",
            grp_dir + "ase200-out.png");
    }
}
