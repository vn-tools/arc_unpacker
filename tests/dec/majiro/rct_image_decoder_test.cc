#include "dec/majiro/rct_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::majiro;

static const std::string dir = "tests/dec/majiro/files/rct/";

static void do_test(
    const std::string &input_path,
    const std::string &expected_path,
    const bstr &key = ""_b)
{
    RctImageDecoder decoder;
    if (!key.empty())
        decoder.set_key(key);
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Majiro RCT images", "[dec]")
{
    SECTION("Version 0")
    {
        do_test("face_dummy.rct", "face_dummy-out.png");
    }

    SECTION("Version 1")
    {
        do_test("ev04_01c.rct", "ev04_01c-out.png");
    }

    SECTION("Encrypted")
    {
        do_test(
            "emocon_21.rct",
            "emocon_21-out.png",
            "\x82\xD6\x82\xDA\x82\xA9\x82\xE9"_b);
    }
}
