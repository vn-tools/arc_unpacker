#include "fmt/leaf/leafpack_group/lc3_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

static const std::string dir = "tests/fmt/leaf/files/lc3/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = Lc3ImageDecoder();
    const auto input_file = tests::file_from_path(dir + input_path);
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Leaf LC3 images", "[fmt]")
{
    do_test("KAO0903.LC3", "KAO0903-out.png");
}
