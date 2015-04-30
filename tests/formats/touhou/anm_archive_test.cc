#include "formats/touhou/anm_archive.h"
#include "test_support/archive_support.h"
#include "test_support/catch.hpp"
#include "test_support/converter_support.h"
using namespace Formats::Touhou;

static void test(
    const std::string path_to_anm,
    const std::vector<std::string> paths_to_png)
{
    std::unique_ptr<Archive> archive(new AnmArchive);
    auto actual_files = unpack_to_memory(path_to_anm, *archive);

    REQUIRE(actual_files.size() == paths_to_png.size());
    for (size_t i = 0; i < paths_to_png.size(); i ++)
    {
        std::unique_ptr<File> expected_file(
            new File(paths_to_png[i], FileIOMode::Read));
        assert_decoded_image(*actual_files[i], *expected_file);
    }
}

TEST_CASE("Decoding format 1 ANM image archives works")
{
    test(
        "tests/formats/touhou/files/eff01.anm",
        { "tests/formats/touhou/files/eff01-out.png" });
}

TEST_CASE("Decoding format 3 ANM image archives works")
{
    test(
        "tests/formats/touhou/files/face_01_00.anm",
        { "tests/formats/touhou/files/face_01_00-out.png" });
}

TEST_CASE("Decoding format 5 ANM image archives works")
{
    test(
        "tests/formats/touhou/files/player00.anm",
        { "tests/formats/touhou/files/player00-out.png" });
}

TEST_CASE("Decoding format 7 ANM image archives works")
{
    test(
        "tests/formats/touhou/files/clouds.anm",
        { "tests/formats/touhou/files/clouds-out.png" });
}

TEST_CASE("Decoding ANM image archives containing multiple sprites works")
{
    test(
        "tests/formats/touhou/files/eff05.anm",
        {
            "tests/formats/touhou/files/eff05-out.png",
            "tests/formats/touhou/files/eff05-out2.png"
        });
}
