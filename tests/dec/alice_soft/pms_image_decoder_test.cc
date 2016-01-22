#include "dec/alice_soft/pms_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::alice_soft;

static const std::string dir = "tests/dec/alice_soft/files/pms/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = PmsImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Alice Soft PMS images", "[dec]")
{
    SECTION("8-bit")
    {
        do_test("CG40000.pm", "CG40000-out.png");
    }

    SECTION("8-bit, inverted channels")
    {
        do_test("ALCG0016.PMS", "ALCG0016-out.png");
    }

    SECTION("16-bit, opaque")
    {
        do_test("G214.PMS", "G214-out.png");
    }

    SECTION("16-bit, transparent")
    {
        do_test("G006.PMS", "G006-out.png");
    }
}
