#include "fmt/alice_soft/ajp_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const std::string dir = "tests/fmt/alice_soft/files/ajp/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const AjpImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_image = tests::image_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Alice Soft AJP images", "[fmt]")
{
    SECTION("Transparent")
    {
        do_test("CG51478.ajp", "CG51478-out.png");
    }
}
