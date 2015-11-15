#include "fmt/twilight_frontier/pak1_image_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

static void do_test(
    const std::string input_path,
    const std::vector<std::string> expected_paths)
{
    const Pak1ImageArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);

    REQUIRE(actual_files.size() == expected_paths.size());
    for (const auto i : util::range(expected_paths.size()))
    {
        const auto expected_image = tests::image_from_path(expected_paths[i]);
        const auto actual_image = tests::image_from_file(*actual_files[i]);
        tests::compare_images(*expected_image, *actual_image);
        REQUIRE(actual_files[i]->name == util::format("%04d.png", i));
    }
}

TEST_CASE("Twilight Frontier PAK1 32-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/twilight_frontier/files/pak1/stage3.dat",
        {
            "tests/fmt/twilight_frontier/files/pak1/stage3-0000-out.png",
            "tests/fmt/twilight_frontier/files/pak1/stage3-0001-out.png",
        });
}

TEST_CASE("Twilight Frontier PAK1 24-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/twilight_frontier/files/pak1/stage10.dat",
        { "tests/fmt/twilight_frontier/files/pak1/stage10-0000-out.png" });
}

TEST_CASE("Twilight Frontier PAK1 16-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/twilight_frontier/files/pak1/effect.dat",
        { "tests/fmt/twilight_frontier/files/pak1/effect-0000-out.png" });
}

TEST_CASE("Twilight Frontier PAK1 8-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/twilight_frontier/files/pak1/07.dat",
        { "tests/fmt/twilight_frontier/files/pak1/07-0000-out.png" });
}
