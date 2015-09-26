#include "fmt/touhou/pak1_image_archive_decoder.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/image_support.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

static void do_test(
    const std::string input_path,
    const std::vector<std::string> expected_paths)
{
    Pak1ImageArchiveDecoder decoder;
    auto actual_files = tests::unpack_to_memory(input_path, decoder);

    REQUIRE(actual_files.size() == expected_paths.size());
    for (auto i : util::range(expected_paths.size()))
    {
        auto expected_image = tests::image_from_path(expected_paths[i]);
        auto actual_image = tests::image_from_file(*actual_files[i]);
        tests::compare_images(*expected_image, *actual_image);
        REQUIRE(actual_files[i]->name == util::format("%04d.png", i));
    }
}

TEST_CASE("Touhou PAK1 32-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/pak1/stage3.dat",
        {
            "tests/fmt/touhou/files/pak1/stage3-0000-out.png",
            "tests/fmt/touhou/files/pak1/stage3-0001-out.png",
        });
}

TEST_CASE("Touhou PAK1 24-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/pak1/stage10.dat",
        { "tests/fmt/touhou/files/pak1/stage10-0000-out.png" });
}

TEST_CASE("Touhou PAK1 16-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/pak1/effect.dat",
        { "tests/fmt/touhou/files/pak1/effect-0000-out.png" });
}

TEST_CASE("Touhou PAK1 8-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/touhou/files/pak1/07.dat",
        { "tests/fmt/touhou/files/pak1/07-0000-out.png" });
}
