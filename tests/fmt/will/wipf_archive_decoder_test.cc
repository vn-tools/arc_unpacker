#include "fmt/will/wipf_archive_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::will;

static void do_test(
    const std::string input_path,
    const std::vector<std::string> expected_paths)
{
    WipfArchiveDecoder decoder;
    auto input_file = tests::file_from_path(input_path);
    auto actual_files = tests::unpack(decoder, *input_file);

    std::vector<std::shared_ptr<pix::Grid>> expected_images;
    for (auto &path : expected_paths)
        expected_images.push_back(tests::image_from_path(path));

    tests::compare_images(expected_images, actual_files);
}

TEST_CASE("Will Co. WIPF 8-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/will/files/wipf/SNOW.MSK",
        { "tests/fmt/will/files/wipf/SNOW-out.png" });
}

TEST_CASE("Will Co. WIPF 24-bit images", "[fmt]")
{
    do_test(
        "tests/fmt/will/files/wipf/CURSOR.WIP",
        { "tests/fmt/will/files/wipf/CURSOR-out.png" });
}

TEST_CASE("Will Co. WIPF multi images", "[fmt]")
{
    do_test(
        "tests/fmt/will/files/wipf/YESNO1.WIP",
        {
            "tests/fmt/will/files/wipf/YESNO1-out-0.png",
            "tests/fmt/will/files/wipf/YESNO1-out-1.png",
            "tests/fmt/will/files/wipf/YESNO1-out-2.png",
            "tests/fmt/will/files/wipf/YESNO1-out-3.png",
            "tests/fmt/will/files/wipf/YESNO1-out-4.png",
        });
}
