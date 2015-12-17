#include "fmt/twilight_frontier/pak1_image_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

static const std::string dir = "tests/fmt/twilight_frontier/files/pak1/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::string> &expected_paths)
{
    const Pak1ImageArchiveDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    std::vector<std::shared_ptr<io::File>> expected_files;
    for (const auto &path : expected_paths)
        expected_files.push_back(tests::file_from_path(dir + path));
    tests::compare_images(expected_files, actual_files, false);
}

TEST_CASE("Twilight Frontier PAK1 images", "[fmt]")
{
    SECTION("32-bit")
    {
        do_test("stage3.dat", {"stage3-0000-out.png", "stage3-0001-out.png"});
    }

    SECTION("24-bit")
    {
        do_test("stage10.dat", {"stage10-0000-out.png"});
    }

    SECTION("16-bit")
    {
        do_test("effect.dat", {"effect-0000-out.png"});
    }

    SECTION("8-bit")
    {
        do_test("07.dat", {"07-0000-out.png"});
    }
}
