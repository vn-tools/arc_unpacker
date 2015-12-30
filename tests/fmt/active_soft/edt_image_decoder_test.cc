#include "fmt/active_soft/edt_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::active_soft;

static const std::string dir = "tests/fmt/active_soft/files/edt/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = EdtImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("ActiveSoft EDT images", "[fmt]")
{
    SECTION("Plain")
    {
        do_test("nowprit.edt", "nowprit-out.png");
    }

    SECTION("Difference-based")
    {
        do_test("nowprit.edt", "nowprit-out.png");
    }
}
