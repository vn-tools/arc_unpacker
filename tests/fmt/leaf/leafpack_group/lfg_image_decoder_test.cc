#include "fmt/leaf/leafpack_group/lfg_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

static const std::string dir = "tests/fmt/leaf/files/lfg/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const LfgImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Leaf LFG images", "[fmt]")
{
    SECTION("Vertical coding")
    {
        do_test("OP_L6.LFG", "OP_L6-out.png");
    }

    SECTION("Horizontal coding")
    {
        do_test("MAX_C09.LFG", "MAX_C09-out.png");
    }
}
