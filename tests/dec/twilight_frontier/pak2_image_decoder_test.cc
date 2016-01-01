#include "dec/twilight_frontier/pak2_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::twilight_frontier;

static const std::string dir = "tests/dec/twilight_frontier/files/pak2/";

static void do_test(
    const std::string &input_path,
    const std::string &expected_path)
{
    const auto decoder = Pak2ImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Twilight Frontier CV2 images", "[dec]")
{
    SECTION("32-bit")
    {
        do_test("0000_00.cv2", "0000_00-out.png");
    }

    SECTION("24-bit")
    {
        do_test("0000_00.cv2", "0000_00-out.png");
    }

    SECTION("8-bit, missing external palette")
    {
        do_test("stand000.cv2", "stand000-out.png");
    }

    SECTION("8-bit, external palette")
    {
        const auto palette_path = dir + "palette000.pal";
        VirtualFileSystem::register_file(
            palette_path,
            [&]()
            {
                return tests::file_from_path(palette_path);
            });

        do_test("stand000.cv2", "stand000-out2.png");
    }
}
