#include "dec/ism/isg_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::ism;

static const std::string dir = "tests/dec/ism/files/isg/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = IsgImageDecoder();
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("ISM ISG images", "[dec]")
{
    SECTION("Variant 16")
    {
        do_test("MUSICTOP-zlib.ISG", "MUSICTOP-out.png");
    }
    SECTION("Variant 21")
    {
        do_test("3D16_019-zlib.ISG", "3D16_019-out.png");
    }
}
