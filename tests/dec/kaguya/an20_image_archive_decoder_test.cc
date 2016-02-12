#include "dec/kaguya/an20_image_archive_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kaguya;

TEST_CASE("Atelier Kaguya AN20 image archives", "[dec]")
{
    const std::vector<res::Image> expected_images =
    {
        tests::get_transparent_test_image(),
        tests::get_opaque_test_image(),
    };
    io::File input_file;
    const std::vector<std::vector<u32>> unk = {
        {0},
        {1, '?', '?'},
        {2, '?'},
        {3, '?'},
        {4, '?'},
        {5, '?'},
    };
    input_file.stream.write("AN20"_b);
    input_file.stream.write_le<u16>(unk.size());
    input_file.stream.write_le<u16>('?');
    for (const auto x : unk)
    {
        input_file.stream.write<u8>(x[0]);
        for (const auto i : algo::range(1, x.size()))
            input_file.stream.write_le<u32>(x[i]);
    }
    input_file.stream.write_le<u16>('?');
    input_file.stream.write_le<u16>(expected_images.size());
    input_file.stream.write_le<u32>(0);
    input_file.stream.write_le<u32>(0);
    input_file.stream.write_le<u32>(expected_images.at(0).height());
    input_file.stream.write_le<u32>(expected_images.at(0).height());
    for (const auto &image : expected_images)
    {
        input_file.stream.write_le<u32>('?');
        input_file.stream.write_le<u32>('?');
        input_file.stream.write_le<u32>(image.width());
        input_file.stream.write_le<u32>(image.height());
        res::Image flipped_image(image);
        flipped_image.flip_vertically();
        if (tests::is_image_transparent(image))
        {
            input_file.stream.write_le<u32>(4);
            tests::write_32_bit_image(input_file.stream, flipped_image);
        }
        else
        {
            input_file.stream.write_le<u32>(3);
            tests::write_24_bit_image(input_file.stream, flipped_image);
        }
    }

    const auto decoder = An20ImageArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_images(actual_files, expected_images);
}
