#include "dec/leaf/ar10_group/cz10_image_archive_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::leaf;

static const std::string dir = "tests/dec/leaf/files/cz10/";

static void do_test(
    const std::string &input_path,
    const std::vector<std::string> &expected_paths)
{
    const auto decoder = Cz10ImageArchiveDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto actual_files = tests::unpack(decoder, *input_file);
    std::vector<std::shared_ptr<io::File>> expected_files;
    for (const auto &path : expected_paths)
        expected_files.push_back(tests::file_from_path(dir + path));
    tests::compare_images(expected_files, actual_files, false);
}

TEST_CASE("Leaf CZ10 images", "[dec]")
{
    SECTION("Encoding type 1+2+3, regular channel order")
    {
        do_test("cbg01_0.cz1", {"cbg01_0-out.png"});
    }

    SECTION("Encoding type 0, multi images")
    {
        do_test(
            "chi06_2.cz1",
            {
                "chi06_2_000-out.png",
                "chi06_2_001-out.png",
                "chi06_2_002-out.png",
                "chi06_2_003-out.png",
                "chi06_2_004-out.png",
            });
    }
}
