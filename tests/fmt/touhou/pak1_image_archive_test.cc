#include "fmt/touhou/pak1_image_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/image_support.h"
#include "util/image.h"

using namespace au;
using namespace au::fmt::touhou;

TEST_CASE("Unpacking PAK1 32-bit images works")
{
    auto expected_image1 = tests::image_from_path(
        "tests/fmt/touhou/files/pak1/stage3-0000-out.png");
    auto expected_image2 = tests::image_from_path(
        "tests/fmt/touhou/files/pak1/stage3-0001-out.png");

    Pak1ImageArchive archive;
    auto actual_files = tests::unpack_to_memory(
        "tests/fmt/touhou/files/pak1/stage3.dat", archive);

    REQUIRE(actual_files.size() == 2);
    REQUIRE(actual_files[0]->name == "0000.png");
    REQUIRE(actual_files[1]->name == "0001.png");
    auto actual_image1 = util::Image::from_boxed(actual_files[0]->io);
    auto actual_image2 = util::Image::from_boxed(actual_files[1]->io);
    tests::compare_images(*expected_image1, *actual_image1);
    tests::compare_images(*expected_image2, *actual_image2);
}

TEST_CASE("Unpacking PAK1 24-bit images works")
{
    auto expected_image = tests::image_from_path(
        "tests/fmt/touhou/files/pak1/stage10-0000-out.png");

    Pak1ImageArchive archive;
    auto actual_files = tests::unpack_to_memory(
        "tests/fmt/touhou/files/pak1/stage10.dat", archive);

    REQUIRE(actual_files.size() == 1);
    REQUIRE(actual_files[0]->name == "0000.png");
    auto actual_image = util::Image::from_boxed(actual_files[0]->io);
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("Unpacking PAK1 16-bit images works")
{
    auto expected_image = tests::image_from_path(
        "tests/fmt/touhou/files/pak1/effect-0000-out.png");

    Pak1ImageArchive archive;
    auto actual_files = tests::unpack_to_memory(
        "tests/fmt/touhou/files/pak1/effect.dat", archive);

    REQUIRE(actual_files.size() == 1);
    REQUIRE(actual_files[0]->name == "0000.png");
    auto actual_image = util::Image::from_boxed(actual_files[0]->io);
    tests::compare_images(*expected_image, *actual_image);
}

TEST_CASE("Unpacking PAK1 8-bit images works")
{
    auto expected_image = tests::image_from_path(
        "tests/fmt/touhou/files/pak1/07-0000-out.png");

    Pak1ImageArchive archive;
    auto actual_files = tests::unpack_to_memory(
        "tests/fmt/touhou/files/pak1/07.dat", archive);

    REQUIRE(actual_files.size() == 1);
    REQUIRE(actual_files[0]->name == "0000.png");
    auto actual_image = util::Image::from_boxed(actual_files[0]->io);
    tests::compare_images(*expected_image, *actual_image);
}
