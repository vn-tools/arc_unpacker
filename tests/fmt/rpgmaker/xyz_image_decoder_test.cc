#include "fmt/rpgmaker/xyz_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::rpgmaker;

static const std::string dir = "tests/fmt/rpgmaker/files/xyz/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = XyzImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("RpgMaker XYZ images", "[fmt]")
{
    do_test("shallows-room-a.xyz", "shallows-room-a-out.png");
}
