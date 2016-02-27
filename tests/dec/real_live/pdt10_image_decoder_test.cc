#include "dec/real_live/pdt10_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::real_live;

static const io::path dir = "tests/dec/real_live/files/pdt10/";

static void do_test(
    const std::string &input_path, const std::string &expected_path)
{
    const auto decoder = Pdt10ImageDecoder();
    const auto input_file = tests::file_from_path(dir / input_path);
    const auto expected_file = tests::file_from_path(dir / expected_path);
    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, *expected_file);
}

TEST_CASE("RealLive PDT10 images", "[dec]")
{
    SECTION("Opaque")
    {
        do_test("SR_L10.PDT", "SR_L10-out.png");
    }

    SECTION("Transparent")
    {
        do_test("KM28B.PDT", "KM28B-out.png");
    }
}
