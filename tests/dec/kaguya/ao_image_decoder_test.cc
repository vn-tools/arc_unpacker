#include "dec/kaguya/ao_image_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kaguya;

TEST_CASE("Kaguya AO images", "[dec]")
{
    const auto decoder = AoImageDecoder();
    const auto input_image = tests::get_transparent_test_image();

    io::File input_file;
    input_file.stream.write("AO");
    input_file.stream.write_le<u32>(input_image.width());
    input_file.stream.write_le<u32>(input_image.height());
    input_file.stream.write_le<u16>(24);
    input_file.stream.write_le<u32>(1);
    input_file.stream.write_le<u32>(2);
    for (const auto y : algo::range(input_image.height() - 1, -1, -1))
    for (const auto x : algo::range(input_image.width()))
    {
        input_file.stream.write<u8>(input_image.at(x, y).b);
        input_file.stream.write<u8>(input_image.at(x, y).g);
        input_file.stream.write<u8>(input_image.at(x, y).r);
        input_file.stream.write<u8>(input_image.at(x, y).a);
    }
    const auto expected_image
        = res::Image(input_image.width() + 1, input_image.height() + 2)
            .overlay(
                input_image,
                1,
                2,
                res::Image::OverlayKind::OverwriteNonTransparent);
    const auto actual_image = tests::decode(decoder, input_file);
    tests::compare_images(actual_image, expected_image);
}
