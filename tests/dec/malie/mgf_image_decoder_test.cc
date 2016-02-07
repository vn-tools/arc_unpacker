#include "dec/malie/mgf_image_decoder.h"
#include "enc/png/png_image_encoder.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::malie;

TEST_CASE("Malie MGF images", "[dec]")
{
    const auto decoder = MgfImageDecoder();
    const auto expected_image = tests::get_transparent_test_image();

    Logger dummy_logger;
    dummy_logger.mute();

    enc::png::PngImageEncoder encoder;
    const auto input_file = encoder.encode(
        dummy_logger, expected_image, "test.mgf");
    input_file->stream.seek(0).write("MalieGF\x00"_b);

    const auto actual_image = tests::decode(decoder, *input_file);
    tests::compare_images(actual_image, expected_image);
}
