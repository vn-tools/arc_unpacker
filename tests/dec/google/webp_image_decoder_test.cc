#include "dec/google/webp_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::google;

static const io::path dir = "tests/dec/google/files/webp/";

static void do_test(
    const std::shared_ptr<io::File> input_file,
    const std::string &expected_path)
{
    const auto decoder = WebpImageDecoder();
    const auto expected_file = tests::file_from_path(dir / expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Google WEBP images", "[dec]")
{
    do_test(tests::file_from_path(dir / "00.webp"), "00-out.png");
}
