#include "fmt/alice_soft/pms_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const std::string dir = "tests/fmt/alice_soft/files/pms/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const PmsImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Alice Soft PMS images", "[fmt]")
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
