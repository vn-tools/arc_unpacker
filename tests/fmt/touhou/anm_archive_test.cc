#include "fmt/touhou/anm_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hh"
#include "test_support/image_support.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

static void test(
    const std::string path_to_anm,
    const std::vector<std::string> paths_to_png)
{
    std::unique_ptr<fmt::Archive> archive(new AnmArchive);
    auto actual_files = tests::unpack_to_memory(path_to_anm, *archive);

    REQUIRE(actual_files.size() == paths_to_png.size());
    for (auto i : util::range(paths_to_png.size()))
    {
        auto expected_image = tests::image_from_path(paths_to_png[i]);
        auto actual_image = util::Image::from_boxed(actual_files[i]->io);
        tests::compare_images(*expected_image, *actual_image);
    }
}

TEST_CASE("Decoding format 1 ANM image archives works")
{
    test(
        "tests/fmt/touhou/files/face_01_00.anm",
        { "tests/fmt/touhou/files/face_01_00-out.png" });
}

TEST_CASE("Decoding format 3 ANM image archives works")
{
    test(
        "tests/fmt/touhou/files/eff01.anm",
        { "tests/fmt/touhou/files/eff01-out.png" });
}

TEST_CASE("Decoding format 5 ANM image archives works")
{
    test(
        "tests/fmt/touhou/files/player00.anm",
        { "tests/fmt/touhou/files/player00-out.png" });
}

TEST_CASE("Decoding format 7 ANM image archives works")
{
    test(
        "tests/fmt/touhou/files/clouds.anm",
        { "tests/fmt/touhou/files/clouds-out.png" });
}

TEST_CASE("Decoding ANM image archives containing multiple sprites works")
{
    test(
        "tests/fmt/touhou/files/eff05.anm",
        {
            "tests/fmt/touhou/files/eff05-out.png",
            "tests/fmt/touhou/files/eff05-out2.png"
        });
}
