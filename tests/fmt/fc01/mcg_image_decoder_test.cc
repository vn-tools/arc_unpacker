#include "fmt/fc01/mcg_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::fc01;

static void do_test(
    const u8 key,
    const std::string &input_path,
    const std::string &expected_path)
{
    McgImageDecoder decoder;
    decoder.set_key(key);
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_image = tests::image_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("FC01 MCG v1.01 24-bit aligned images", "[fmt]")
{
    do_test(
        209,
        "tests/fmt/fc01/files/mcg/FC01LOGO.MCG",
        "tests/fmt/fc01/files/mcg/FC01LOGO-out.png");
}

TEST_CASE("FC01 MCG v1.01 24-bit unaligned images", "[fmt]")
{
    do_test(
        209,
        "tests/fmt/fc01/files/mcg/CLE13_06.MCG",
        "tests/fmt/fc01/files/mcg/CLE13_06-out.png");
}

TEST_CASE("FC01 MCG v2.00 24-bit images", "[fmt]")
{
    do_test(
        77,
        "tests/fmt/fc01/files/mcg/50.MCG",
        "tests/fmt/fc01/files/mcg/50-out.png");
}
