#include "fmt/majiro/rct_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::majiro;

static void do_test(
    const std::string &input_path,
    const std::string &expected_path,
    const bstr &key = ""_b)
{
    RctImageDecoder decoder;
    if (!key.empty())
        decoder.set_key(key);
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_image = tests::image_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Majiro RCT v0 images", "[fmt]")
{
    do_test(
        "tests/fmt/majiro/files/rct/face_dummy.rct",
        "tests/fmt/majiro/files/rct/face_dummy-out.png");
}

TEST_CASE("Majiro RCT v1 images", "[fmt]")
{
    do_test(
        "tests/fmt/majiro/files/rct/ev04_01c.rct",
        "tests/fmt/majiro/files/rct/ev04_01c-out.png");
}

TEST_CASE("Majiro RCT encrypted images", "[fmt]")
{
    do_test(
        "tests/fmt/majiro/files/rct/emocon_21.rct",
        "tests/fmt/majiro/files/rct/emocon_21-out.png",
        "\x82\xD6\x82\xDA\x82\xA9\x82\xE9"_b);
}
