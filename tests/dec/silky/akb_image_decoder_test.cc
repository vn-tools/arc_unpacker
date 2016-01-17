#include "dec/silky/akb_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::silky;

static const std::string dir = "tests/dec/silky/files/akb/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = AkbImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Silky AKB images", "[dec]")
{
    SECTION("24-bit")
    {
        do_test("HINT02.AKB", "HINT02-out.png");
    }

    SECTION("32-bit")
    {
        do_test("BREATH.AKB", "BREATH-out.png");
    }
}
