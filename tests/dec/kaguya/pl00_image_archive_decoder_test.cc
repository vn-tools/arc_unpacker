#include "dec/kaguya/pl00_image_archive_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kaguya;

TEST_CASE("Atelier Kaguya PL00 image archives", "[dec]")
{
    const std::vector<res::Image> expected_images =
    {
        tests::get_opaque_test_image(),
        tests::get_transparent_test_image(),
    };

    io::File input_file;
    input_file.stream.write("PL00"_b);
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
        bool transparent = false;
        for (const auto &c : image)
            if (c.a != 0xFF)
                transparent = true;
        if (transparent)
        {
            input_file.stream.write_le<u32>(4);
            for (const auto y : algo::range(image.height() - 1, -1, -1))
            for (const auto x : algo::range(image.width()))
            {
                input_file.stream.write<u8>(image.at(x, y).b);
                input_file.stream.write<u8>(image.at(x, y).g);
                input_file.stream.write<u8>(image.at(x, y).r);
                input_file.stream.write<u8>(image.at(x, y).a);
            }
        }
        else
        {
            input_file.stream.write_le<u32>(3);
            for (const auto y : algo::range(image.height() - 1, -1, -1))
            for (const auto x : algo::range(image.width()))
            {
                input_file.stream.write<u8>(image.at(x, y).b);
                input_file.stream.write<u8>(image.at(x, y).g);
                input_file.stream.write<u8>(image.at(x, y).r);
            }
        }
    }

    const auto decoder = Pl00ImageArchiveDecoder();
    const auto actual_files = tests::unpack(decoder, input_file);
    tests::compare_images(actual_files, expected_images);
}
