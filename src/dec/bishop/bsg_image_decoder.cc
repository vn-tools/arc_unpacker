#include "dec/bishop/bsg_image_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::bishop;

static const bstr magic = "BSS-Graphics"_b;

bool BsgImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

static void unpack_none(
    io::BaseByteStream &input_stream,
    algo::ptr<u8> output_ptr,
    const size_t data_size,
    const size_t stride)
{
    bstr input = input_stream.read(data_size);
    for (const auto i : algo::range(input.size()))
    {
        if (!output_ptr.left())
            break;
        *output_ptr = input[i];
        output_ptr += stride;
    }
}

static void unpack_rle(
    io::BaseByteStream &input_stream,
    algo::ptr<u8> output_ptr,
    const size_t data_size,
    const size_t stride)
{
    auto remaining = input_stream.read_le<u32>();
    while (remaining)
    {
        const auto count = input_stream.read<s8>();
        --remaining;
        if (count >= 0)
        {
            for (const auto i : algo::range(1 + count))
            {
                if (!output_ptr.left())
                    break;
                *output_ptr = input_stream.read<u8>();
                output_ptr += stride;
                --remaining;
            }
        }
        else
        {
            const auto repeat = input_stream.read<u8>();
            --remaining;
            for (const auto i : algo::range(1 - count))
            {
                if (!output_ptr.left())
                    throw err::BadDataSizeError();
                *output_ptr = repeat;
                output_ptr += stride;
            }
        }
    }
}


res::Image BsgImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0x12);
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();
    input_file.stream.skip(6);
    const auto x = input_file.stream.read_le<u16>();
    const auto y = input_file.stream.read_le<u16>();
    input_file.stream.skip(12);
    const auto color_type = input_file.stream.read<u8>();
    const auto compression_type = input_file.stream.read<u8>();
    const auto data_offset = input_file.stream.read_le<u32>();
    const auto data_size = input_file.stream.read_le<u32>();
    const auto palette_offset = input_file.stream.read_le<u32>();

    void (*channel_unpacker)(
        io::BaseByteStream &, algo::ptr<u8>, const size_t, const size_t);

    if (compression_type == 0)
        channel_unpacker = unpack_none;
    else if (compression_type == 1)
        channel_unpacker = unpack_rle;
    else
        throw err::NotSupportedError("Unknown compression type");

    std::unique_ptr<res::Image> image;

    input_file.stream.seek(data_offset);
    if (color_type == 0)
    {
        bstr output(width * height * 4);
        auto output_ptr = algo::make_ptr(output);
        for (const auto i : algo::range(4))
        {
            channel_unpacker(
                input_file.stream, output_ptr + i, data_size / 4, 4);
        }
        image = std::make_unique<res::Image>(
            width, height, output, res::PixelFormat::BGRA8888);
    }
    else if (color_type == 1)
    {
        bstr output(width * height * 3);
        auto output_ptr = algo::make_ptr(output);
        for (const auto i : algo::range(3))
        {
            channel_unpacker(
                input_file.stream, output_ptr + i, data_size / 3, 3);
        }
        image = std::make_unique<res::Image>(
            width, height, output, res::PixelFormat::BGR888);
    }
    else if (color_type == 2)
    {
        bstr output(width * height);
        auto output_ptr = algo::make_ptr(output);
        channel_unpacker(input_file.stream, output_ptr, data_size, 1);

        if (palette_offset)
        {
            res::Palette palette(
                256,
                input_file.stream.seek(palette_offset),
                res::PixelFormat::BGRA8888);
            image = std::make_unique<res::Image>(
                width, height, output, palette);
        }
        else
        {
            image = std::make_unique<res::Image>(
                width, height, output, res::PixelFormat::Gray8);
        }
    }
    else
        throw err::NotSupportedError("Unsupported image parameters");

    image->flip_vertically();
    return *image;
}

static auto _ = dec::register_decoder<BsgImageDecoder>("bishop/bsg");
