#include "fmt/libido/mnc_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::libido;

static const std::string dir = "tests/fmt/libido/files/mnc/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const MncImageDecoder decoder;
    const auto input_file = tests::zlib_file_from_path(dir + input_path);
    const auto expected_image = tests::image_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_image, actual_image);
}

TEST_CASE("Libido MNC images", "[fmt]")
{
    do_test("test-zlib.MNC", "test-out.png");
}
