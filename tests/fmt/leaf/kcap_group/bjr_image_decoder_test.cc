#include "fmt/leaf/kcap_group/bjr_image_decoder.h"
#include "test_support/catch.hh"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::fmt::leaf;

static const std::string dir = "tests/fmt/leaf/files/bjr/";

static void do_test(
    const std::shared_ptr<io::File> input_file,
    const std::string &expected_path)
{
    const BjrImageDecoder decoder;
    const auto expected_file = tests::file_from_path(dir + expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(*expected_file, actual_image);
}

TEST_CASE("Leaf BJR images", "[fmt]")
{
    do_test(
        tests::zlib_file_from_path(dir + "v00232-zlib.BJR", "v00232.BJR"),
        "v00232-out.png");
}
