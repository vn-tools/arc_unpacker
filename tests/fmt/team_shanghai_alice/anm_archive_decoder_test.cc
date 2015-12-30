#include "fmt/team_shanghai_alice/anm_archive_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::team_shanghai_alice;

static const std::string dir = "tests/fmt/team_shanghai_alice/files/anm/";

static void do_test(
    const std::string input_path,
    const std::vector<std::string> expected_paths)
{
    const auto decoder = AnmArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    std::vector<std::shared_ptr<io::File>> expected_files;
    for (const auto &path : expected_paths)
        expected_files.push_back(tests::file_from_path(dir + path));
    tests::compare_images(expected_files, actual_files, false);
}

TEST_CASE("Team Shanghai Alice ANM sprite containers", "[fmt]")
{
    SECTION("Format 1")
    {
        do_test("face_01_00.anm", {"face_01_00-out.png"});
    }

    SECTION("Format 3")
    {
        do_test("eff01.anm", {"eff01-out.png"});
    }

    SECTION("Format 5")
    {
        do_test("player00.anm", {"player00-out.png"});
    }

    SECTION("Format 7")
    {
        do_test("clouds.anm", {"clouds-out.png"});
    }

    SECTION("Multi images")
    {
        do_test("eff05.anm", {"eff05-out.png", "eff05-out2.png"});
    }
}
