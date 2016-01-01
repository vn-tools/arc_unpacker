#include "dec/leaf/leafpack_group/lfb_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::leaf;

static const std::string dir = "tests/dec/leaf/files/lfb/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = LfbImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Leaf LFB images", "[dec]")
{
    do_test("LIFEM.LFB", "LIFEM-out.png");
}
