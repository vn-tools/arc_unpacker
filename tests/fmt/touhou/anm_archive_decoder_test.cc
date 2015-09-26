#include "fmt/touhou/anm_archive_decoder.h"
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
    AnmArchiveDecoder decoder;
    auto actual_files = tests::unpack_to_memory(input_path, decoder);

    REQUIRE(actual_files.size() == expected_paths.size());
    for (auto i : util::range(expected_paths.size()))
    {
        auto expected_image = tests::image_from_path(expected_paths[i]);
        auto actual_image = tests::image_from_file(*actual_files[i]);
        tests::compare_images(*expected_image, *actual_image);
    }
}

TEST_CASE("Touhou ANM format 1 sprite containers", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/anm-1/face_01_00.anm",
        { "tests/fmt/touhou/files/anm-1/face_01_00-out.png" });
}

TEST_CASE("Touhou ANM format 3 sprite containers", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/anm-3/eff01.anm",
        { "tests/fmt/touhou/files/anm-3/eff01-out.png" });
}

TEST_CASE("Touhou ANM format 5 sprite containers", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/anm-5/player00.anm",
        { "tests/fmt/touhou/files/anm-5/player00-out.png" });
}

TEST_CASE("Touhou ANM format 7 sprite containers", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/anm-7/clouds.anm",
        { "tests/fmt/touhou/files/anm-7/clouds-out.png" });
}

TEST_CASE("Touhou ANM multi sprite containers", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/anm-multi/eff05.anm",
        {
            "tests/fmt/touhou/files/anm-multi/eff05-out.png",
            "tests/fmt/touhou/files/anm-multi/eff05-out2.png"
        });
}
