#include "fmt/team_shanghai_alice/anm_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::team_shanghai_alice;

static void do_test(
    const std::string input_path,
    const std::vector<std::string> expected_paths)
{
    const AnmArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);

    REQUIRE(actual_files.size() == expected_paths.size());
    for (const auto i : util::range(expected_paths.size()))
    {
        const auto expected_image = tests::image_from_path(expected_paths[i]);
        const auto actual_image = tests::image_from_file(*actual_files[i]);
        tests::compare_images(*expected_image, *actual_image);
    }
}

TEST_CASE("Team Shanghai Alice ANM format 1 sprite containers", "[fmt]")
{
    do_test(
        "tests/fmt/team_shanghai_alice/files/anm-1/face_01_00.anm",
        { "tests/fmt/team_shanghai_alice/files/anm-1/face_01_00-out.png" });
}

TEST_CASE("Team Shanghai Alice ANM format 3 sprite containers", "[fmt]")
{
    do_test(
        "tests/fmt/team_shanghai_alice/files/anm-3/eff01.anm",
        { "tests/fmt/team_shanghai_alice/files/anm-3/eff01-out.png" });
}

TEST_CASE("Team Shanghai Alice ANM format 5 sprite containers", "[fmt]")
{
    do_test(
        "tests/fmt/team_shanghai_alice/files/anm-5/player00.anm",
        { "tests/fmt/team_shanghai_alice/files/anm-5/player00-out.png" });
}

TEST_CASE("Team Shanghai Alice ANM format 7 sprite containers", "[fmt]")
{
    do_test(
        "tests/fmt/team_shanghai_alice/files/anm-7/clouds.anm",
        { "tests/fmt/team_shanghai_alice/files/anm-7/clouds-out.png" });
}

TEST_CASE("Team Shanghai Alice ANM multi sprite containers", "[fmt]")
{
    do_test(
        "tests/fmt/team_shanghai_alice/files/anm-multi/eff05.anm",
        {
            "tests/fmt/team_shanghai_alice/files/anm-multi/eff05-out.png",
            "tests/fmt/team_shanghai_alice/files/anm-multi/eff05-out2.png"
        });
}
