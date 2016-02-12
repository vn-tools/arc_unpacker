#include "dec/kaguya/anm_image_archive_decoder.h"
#include "algo/range.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/file_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::kaguya;

static bool is_transparent(const res::Image &image)
{
    for (const auto &c : image)
        if (c.a != 0xFF)
            return true;
    return false;
}

static void write_32_bit_image(
    io::BaseByteStream &output_stream, const res::Image &image)
{
    for (const auto y : algo::range(image.height() - 1, -1, -1))
    for (const auto x : algo::range(image.width()))
    {
        output_stream.write<u8>(image.at(x, y).b);
        output_stream.write<u8>(image.at(x, y).g);
        output_stream.write<u8>(image.at(x, y).r);
        output_stream.write<u8>(image.at(x, y).a);
    }
}

static void write_24_bit_image(
    io::BaseByteStream &output_stream, const res::Image &image)
{
    for (const auto y : algo::range(image.height() - 1, -1, -1))
    for (const auto x : algo::range(image.width()))
    {
        output_stream.write<u8>(image.at(x, y).b);
        output_stream.write<u8>(image.at(x, y).g);
        output_stream.write<u8>(image.at(x, y).r);
    }
}

static void write_an00(
    io::BaseByteStream &output_stream, const std::vector<res::Image> &images)
{
    output_stream.write("AN00"_b);
    output_stream.write_le<u32>(0);
    output_stream.write_le<u32>(0);
    output_stream.write_le<u32>(images.at(0).height());
    output_stream.write_le<u32>(images.at(0).height());
    const std::vector<u32> unk = {1, 2, 3, 4};
    output_stream.write_le<u16>(unk.size());
    output_stream.write_le<u16>('?');
    for (const auto x : unk)
        output_stream.write_le<u32>(x);
    output_stream.write_le<u16>(images.size());
    for (const auto &image : images)
    {
        output_stream.write_le<u32>('?');
        output_stream.write_le<u32>('?');
        output_stream.write_le<u32>(image.width());
        output_stream.write_le<u32>(image.height());
        write_32_bit_image(output_stream, image);
    }
}

static void write_an10(
    io::BaseByteStream &output_stream, const std::vector<res::Image> &images)
{
    output_stream.write("AN10"_b);
    output_stream.write_le<u32>(0);
    output_stream.write_le<u32>(0);
    output_stream.write_le<u32>(images.at(0).height());
    output_stream.write_le<u32>(images.at(0).height());
    const std::vector<u32> unk = {1, 2, 3, 4};
    output_stream.write_le<u16>(unk.size());
    output_stream.write_le<u16>('?');
    for (const auto x : unk)
        output_stream.write_le<u32>(x);
    output_stream.write_le<u16>(images.size());
    for (const auto &image : images)
    {
        output_stream.write_le<u32>('?');
        output_stream.write_le<u32>('?');
        output_stream.write_le<u32>(image.width());
        output_stream.write_le<u32>(image.height());
        if (is_transparent(image))
        {
            output_stream.write_le<u32>(4);
            write_32_bit_image(output_stream, image);
        }
        else
        {
            output_stream.write_le<u32>(3);
            write_24_bit_image(output_stream, image);
        }
    }
}

static void write_an20(
    io::BaseByteStream &output_stream, const std::vector<res::Image> &images)
{
    const std::vector<std::vector<u32>> unk = {
        {0},
        {1, '?', '?'},
        {2, '?'},
        {3, '?'},
        {4, '?'},
        {5, '?'},
    };
    output_stream.write("AN20"_b);
    output_stream.write_le<u16>(unk.size());
    output_stream.write_le<u16>('?');
    for (const auto x : unk)
    {
        output_stream.write<u8>(x[0]);
        for (const auto i : algo::range(1, x.size()))
            output_stream.write_le<u32>(x[i]);
    }
    output_stream.write_le<u16>('?');
    output_stream.write_le<u16>(images.size());
    output_stream.write_le<u32>(0);
    output_stream.write_le<u32>(0);
    output_stream.write_le<u32>(images.at(0).height());
    output_stream.write_le<u32>(images.at(0).height());
    for (const auto &image : images)
    {
        output_stream.write_le<u32>('?');
        output_stream.write_le<u32>('?');
        output_stream.write_le<u32>(image.width());
        output_stream.write_le<u32>(image.height());
        if (is_transparent(image))
        {
            output_stream.write_le<u32>(4);
            write_32_bit_image(output_stream, image);
        }
        else
        {
            output_stream.write_le<u32>(3);
            write_24_bit_image(output_stream, image);
        }
    }
}

TEST_CASE("Atelier Kaguya ANM archives", "[dec]")
{
    const auto decoder = AnmImageArchiveDecoder();

    SECTION("AN00")
    {
        const std::vector<res::Image> expected_images =
        {
            tests::get_transparent_test_image(),
            tests::get_opaque_test_image(),
        };
        io::File input_file;
        write_an00(input_file.stream, expected_images);
        const auto actual_files = tests::unpack(decoder, input_file);
        tests::compare_images(actual_files, expected_images);
    }

    SECTION("AN10")
    {
        const std::vector<res::Image> expected_images =
        {
            tests::get_transparent_test_image(),
            tests::get_opaque_test_image(),
        };
        io::File input_file;
        write_an10(input_file.stream, expected_images);
        const auto actual_files = tests::unpack(decoder, input_file);
        tests::compare_images(actual_files, expected_images);
    }

    SECTION("AN20")
    {
        const std::vector<res::Image> expected_images =
        {
            tests::get_transparent_test_image(),
            tests::get_opaque_test_image(),
        };
        io::File input_file;
        write_an20(input_file.stream, expected_images);
        const auto actual_files = tests::unpack(decoder, input_file);
        tests::compare_images(actual_files, expected_images);
    }
}
