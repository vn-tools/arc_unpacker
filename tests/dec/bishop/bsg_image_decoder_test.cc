#include "dec/bishop/bsg_image_decoder.h"
#include "algo/range.h"
#include "io/memory_stream.h"
#include "test_support/catch.h"
#include "test_support/decoder_support.h"
#include "test_support/image_support.h"

using namespace au;
using namespace au::dec::bishop;

static bstr compress_rle(const bstr &input)
{
    io::MemoryStream output_stream;
    io::MemoryStream input_stream(input);
    input_stream.seek(0);
    while (input_stream.left())
    {
        auto init_b = input_stream.read<u8>();
        if (!input_stream.left())
        {
            output_stream.write<u8>(0);
            output_stream.write<u8>(init_b);
            break;
        }

        auto other_b = input_stream.read<u8>();
        if (other_b == init_b)
        {
            auto repetitions = 2;
            while (input_stream.left() && repetitions < 128)
            {
                auto further_b = input_stream.read<u8>();
                if (further_b != init_b)
                {
                    input_stream.skip(-1);
                    break;
                }
                ++repetitions;
            }
            output_stream.write<s8>(-(repetitions - 1));
            output_stream.write<u8>(init_b);
        }
        else
        {
            bstr chunk;
            chunk += init_b;
            chunk += other_b;
            while (input_stream.left() && chunk.size() < 128)
            {
                auto further_b = input_stream.read<u8>();
                if (further_b == init_b)
                {
                    input_stream.skip(-1);
                    break;
                }
                chunk += further_b;
            }
            output_stream.write<s8>(chunk.size() - 1);
            output_stream.write(chunk);
        }
    }
    return output_stream.seek(0).read_to_eof();
}

static void write_header(
    io::BaseByteStream &output_stream,
    const res::Image &image,
    const u8 color_type,
    const u8 compression_type)
{
    output_stream.write("BSS-Graphics"_b);
    output_stream.write("????"_b);
    output_stream.write("??"_b);
    output_stream.write("JUNK"_b);
    output_stream.write_le<u16>(image.width());
    output_stream.write_le<u16>(image.height());
    output_stream.write("????"_b);
    output_stream.write("??"_b);
    output_stream.write_le<u16>(0);
    output_stream.write_le<u16>(0);
    output_stream.write("????"_b);
    output_stream.write("????"_b);
    output_stream.write("????"_b);
    output_stream.write<u8>(color_type);
    output_stream.write<u8>(compression_type);
}

TEST_CASE("Bishop BSG images", "[dec]")
{
    const auto decoder = BsgImageDecoder();
    io::File input_file;
    res::Image expected_image(1, 1);

    SECTION("Uncompressed RGBA")
    {
        expected_image = tests::get_transparent_test_image();
        write_header(
            input_file.stream, expected_image, 0, 0);
        const auto data_offset_stub = input_file.stream.pos();
        input_file.stream.write("STUB"_b);
        input_file.stream.write_le<u32>(
            expected_image.width() * expected_image.height() * 4);
        input_file.stream.write_le<u32>(0);
        const auto data_offset = input_file.stream.pos();
        auto tmp_image = expected_image;
        tmp_image.flip_vertically();
        for (const auto &p : tmp_image) input_file.stream.write<u8>(p.b);
        for (const auto &p : tmp_image) input_file.stream.write<u8>(p.g);
        for (const auto &p : tmp_image) input_file.stream.write<u8>(p.r);
        for (const auto &p : tmp_image) input_file.stream.write<u8>(p.a);
        input_file.stream.seek(data_offset_stub).write_le<u32>(data_offset);
    }

    SECTION("Uncompressed, RGB")
    {
        expected_image = tests::get_opaque_test_image();
        write_header(
            input_file.stream, expected_image, 1, 0);
        const auto data_offset_stub = input_file.stream.pos();
        input_file.stream.write("STUB"_b);
        input_file.stream.write_le<u32>(
            expected_image.width() * expected_image.height() * 3);
        input_file.stream.write_le<u32>(0);
        const auto data_offset = input_file.stream.pos();
        auto tmp_image = expected_image;
        tmp_image.flip_vertically();
        for (const auto &p : tmp_image) input_file.stream.write<u8>(p.b);
        for (const auto &p : tmp_image) input_file.stream.write<u8>(p.g);
        for (const auto &p : tmp_image) input_file.stream.write<u8>(p.r);
        input_file.stream.seek(data_offset_stub).write_le<u32>(data_offset);
    }

    SECTION("Uncompressed, palette")
    {
        const auto palette_result = tests::get_palette_test_image();
        expected_image = std::get<0>(palette_result);
        const auto palette_indices = std::get<1>(palette_result);
        const auto input_palette = std::get<2>(palette_result);
        write_header(
            input_file.stream, expected_image, 2, 0);
        const auto data_offset_stub = input_file.stream.pos();
        input_file.stream.write("STUB"_b);
        input_file.stream.write_le<u32>(
            expected_image.width() * expected_image.height());
        const auto palette_offset_stub = input_file.stream.pos();
        input_file.stream.write("STUB"_b);
        const auto data_offset = input_file.stream.pos();
        for (const auto y : algo::range(expected_image.height() - 1, -1, -1))
        for (const auto x : algo::range(expected_image.width()))
            input_file.stream.write<u8>(palette_indices.at(x, y));
        const auto palette_offset = input_file.stream.pos();
        for (const auto i : algo::range(input_palette.size()))
        {
            input_file.stream.write<u8>(input_palette[i].b);
            input_file.stream.write<u8>(input_palette[i].g);
            input_file.stream.write<u8>(input_palette[i].r);
            input_file.stream.write<u8>(input_palette[i].a);
        }
        input_file.stream.seek(data_offset_stub).write_le<u32>(data_offset);
        input_file
            .stream.seek(palette_offset_stub)
            .write_le<u32>(palette_offset);
    }

    SECTION("RLE-compressed, RGB")
    {
        expected_image = tests::get_opaque_test_image();
        write_header(input_file.stream, expected_image, 1, 1);
        const auto data_offset_stub = input_file.stream.pos();
        input_file.stream.write("STUB"_b);
        input_file.stream.write_le<u32>(
            expected_image.width() * expected_image.height() * 3);
        input_file.stream.write_le<u32>(0);
        const auto data_offset = input_file.stream.pos();
        auto tmp_image = expected_image;
        tmp_image.flip_vertically();
        for (const auto i : algo::range(3))
        {
            io::MemoryStream chunk_stream;
            for (const auto &p : tmp_image)
                chunk_stream.write<u8>(p[i]);
            const auto compressed
                = compress_rle(chunk_stream.seek(0).read_to_eof());
            input_file.stream.write_le<u32>(compressed.size());
            input_file.stream.write(compressed);
        }
        input_file.stream.seek(data_offset_stub).write_le<u32>(data_offset);
    }

    const auto actual_image = tests::decode(decoder, input_file);
    tests::compare_images(actual_image, expected_image);
}
