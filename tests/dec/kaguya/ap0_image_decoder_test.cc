#include "dec/kaguya/ap0_image_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kaguya;

TEST_CASE("Kaguya AP0 images", "[dec]")
{
    const auto decoder = Ap0ImageDecoder();
    const auto input_image = tests::get_transparent_test_image();

    io::File input_file;
    input_file.stream.write("AP-0");
    input_file.stream.write_le<u32>(input_image.width());
    input_file.stream.write_le<u32>(input_image.height());
    for (const auto y : algo::range(input_image.height() - 1, -1, -1))
    for (const auto x : algo::range(input_image.width()))
        input_file.stream.write<u8>(input_image.at(x, y).a);

    auto expected_image = input_image;
    for (const auto y : algo::range(input_image.height() - 1, -1, -1))
    for (const auto x : algo::range(input_image.width()))
    {
        res::Pixel &pixel = expected_image.at(x, y);
        pixel.r = pixel.a;
        pixel.g = pixel.a;
        pixel.b = pixel.a;
        pixel.a = 0xFF;
    }
    const auto actual_image = tests::decode(decoder, input_file);
    tests::compare_images(actual_image, expected_image);
}
