#include "dec/alice_soft/dcf_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::alice_soft;

static const std::string dcf_dir = "tests/dec/alice_soft/files/dcf/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = DcfImageDecoder();
    const auto input_file = tests::file_from_path(input_path);
    const auto expected_file = tests::file_from_path(expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("Alice Soft DCF images", "[dec]")
{
    do_test(dcf_dir + "tere.dcf", dcf_dir + "tere-out.png");
}
