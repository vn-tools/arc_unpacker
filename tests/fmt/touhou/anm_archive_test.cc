#include "fmt/touhou/anm_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/image_support.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

static void do_test(
    const std::string input_path,
    const std::vector<std::string> expected_paths)
{
    AnmArchive archive;
    auto actual_files = tests::unpack_to_memory(input_path, archive);

    REQUIRE(actual_files.size() == expected_paths.size());
    for (auto i : util::range(expected_paths.size()))
    {
        auto expected_image = tests::image_from_path(expected_paths[i]);
        auto actual_image = util::Image::from_boxed(actual_files[i]->io);
        tests::compare_images(*expected_image, *actual_image);
    }
}

TEST_CASE("Decoding format 1 ANM image archives works", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/anm-1/face_01_00.anm",
        { "tests/fmt/touhou/files/anm-1/face_01_00-out.png" });
}

TEST_CASE("Decoding format 3 ANM image archives works", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/anm-3/eff01.anm",
        { "tests/fmt/touhou/files/anm-3/eff01-out.png" });
}

TEST_CASE("Decoding format 5 ANM image archives works", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/anm-5/player00.anm",
        { "tests/fmt/touhou/files/anm-5/player00-out.png" });
}

TEST_CASE("Decoding format 7 ANM image archives works", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/anm-7/clouds.anm",
        { "tests/fmt/touhou/files/anm-7/clouds-out.png" });
}

TEST_CASE(
    "Decoding ANM image archives containing multiple sprites works", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/anm-multi/eff05.anm",
        {
            "tests/fmt/touhou/files/anm-multi/eff05-out.png",
            "tests/fmt/touhou/files/anm-multi/eff05-out2.png"
        });
}
