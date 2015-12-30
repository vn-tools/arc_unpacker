#include "fmt/alice_soft/qnt_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const std::string dir = "tests/fmt/alice_soft/files/qnt/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = QntImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Alice Soft QNT images", "[fmt]")
{
    SECTION("Opaque")
    {
        do_test("CG00505.QNT", "CG00505-out.png");
    }

    SECTION("Transparent images with resolution of 2^n")
    {
        do_test("CG64100.QNT", "CG64100-out.png");
    }

    SECTION("Transparent images with arbitrary resolution")
    {
        do_test("CG64214.QNT", "CG64214-out.png");
    }

    SECTION("Transparency data only")
    {
        do_test("cg50121.QNT", "cg50121-out.png");
    }
}
