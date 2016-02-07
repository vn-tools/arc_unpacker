#include "dec/kaguya/bmr_file_decoder.h"
#include "dec/microsoft/bmp_image_decoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kaguya;

static const io::path dir = "tests/dec/kaguya/files/bmr/";

TEST_CASE("Atelier Kaguya BMR files", "[dec]")
{
    const auto bmr_decoder = BmrFileDecoder();
    const auto input_file = tests::file_from_path(dir / "bg_black.bmp");
    const auto expected_file = tests::file_from_path(dir / "bg_black-out.png");
    const auto actual_file = tests::decode(bmr_decoder, *input_file);
    const auto bmp_decoder = dec::microsoft::BmpImageDecoder();
    const auto actual_image = tests::decode(bmp_decoder, *actual_file);
    tests::compare_images(actual_image, *expected_file);
}
