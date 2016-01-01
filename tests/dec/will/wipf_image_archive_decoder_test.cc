#include "dec/will/wipf_image_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::will;

static const std::string dir = "tests/dec/will/files/wipf/";

static void do_test(
    const std::string input_path,
    const std::vector<std::string> expected_paths)
{
    const auto decoder = WipfImageArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    std::vector<std::shared_ptr<io::File>> expected_files;
    for (const auto &path : expected_paths)
        expected_files.push_back(tests::file_from_path(dir + path));
    tests::compare_images(expected_files, actual_files, false);
}

TEST_CASE("Will Co. WIPF images", "[dec]")
{
    SECTION("8-bit")
    {
        do_test("SNOW.MSK", {"SNOW-out.png"});
    }

    SECTION("24-bit")
    {
        do_test("CURSOR.WIP", {"CURSOR-out.png"});
    }

    SECTION("Multi images")
    {
        do_test(
            "YESNO1.WIP",
            {
                "YESNO1-out-0.png",
                "YESNO1-out-1.png",
                "YESNO1-out-2.png",
                "YESNO1-out-3.png",
                "YESNO1-out-4.png",
            });
    }
}
