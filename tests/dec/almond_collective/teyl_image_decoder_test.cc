#include "dec/almond_collective/teyl_image_decoder.h"
#include "algo/range.h"
#include "io/memory_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::almond_collective;

TEST_CASE("Almond Collective TEYL images", "[dec]")
{
    const auto decoder = TeylImageDecoder();
    const auto input_image = tests::get_transparent_test_image();

    io::File input_file;
    input_file.stream.write("TEYL"_b);
    input_file.stream.write_le<u32>(input_image.width());
    input_file.stream.write_le<u32>(input_image.height());
    for (const auto y : algo::range(input_image.height()))
    for (const auto x : algo::range(input_image.width()))
    {
        input_file.stream.write<u8>(input_image.at(x, y).b);
        input_file.stream.write<u8>(input_image.at(x, y).g);
        input_file.stream.write<u8>(input_image.at(x, y).r);
        input_file.stream.write<u8>(input_image.at(x, y).a);
    }

    const auto expected_image = input_image;
    const auto actual_image = tests::decode(decoder, input_file);
    tests::compare_images(actual_image, expected_image);
}
