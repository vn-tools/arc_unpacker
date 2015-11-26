#include "fmt/amuse_craft/pgd_ge_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::amuse_craft;

static const std::string dir = "tests/fmt/amuse_craft/files/pgd-ge/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const PgdGeImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Amuse Craft PGD images (GE)", "[fmt]")
{
    SECTION("24-bit, filter 2")
    {
        do_test("ETLA023R.PGD", "ETLA023R-out.png");
    }

    SECTION("24-bit / filter 3")
    {
        do_test("SECRET.PGD", "SECRET-out.png");
    }

    SECTION("32-bit / filter 3")
    {
        do_test("SYSTEM.PGD", "SYSTEM-out.png");
    }
}
