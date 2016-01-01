#include "dec/kirikiri/tlg_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kirikiri;

static const std::string dir = "tests/dec/kirikiri/files/tlg/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = TlgImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("KiriKiri TLG images", "[dec]")
{
    SECTION("TLG5")
    {
        do_test("14.tlg", "14-out.png");
    }

    SECTION("TLG6")
    {
        do_test("tlg6.tlg", "tlg6-out.png");
    }

    SECTION("TLG0")
    {
        do_test("bg08d.tlg", "bg08d-out.png");
    }
}
