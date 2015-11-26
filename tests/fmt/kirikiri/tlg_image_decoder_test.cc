#include "fmt/kirikiri/tlg_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::kirikiri;

static const std::string dir = "tests/fmt/kirikiri/files/tlg/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const TlgImageDecoder decoder;
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("KiriKiri TLG images", "[fmt]")
{
    SECTION("TLG5")
    {
        do_test("14凛ペンダント.tlg", "14凛ペンダント-out.png");
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
