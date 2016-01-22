#include "dec/crowd/zbm_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::crowd;

static const std::string dir = "tests/dec/crowd/files/zbm/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = ZbmImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Crowd ZBM images", "[dec]")
{
    do_test("Takuf_26.zbm", "Takuf_26-out.png");
}
