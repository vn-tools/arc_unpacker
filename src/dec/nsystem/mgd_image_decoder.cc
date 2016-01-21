#include "dec/nsystem/mgd_image_decoder.h"
#include "algo/range.h"
#include "dec/png/png_image_decoder.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::nsystem;

namespace
{
    enum class CompressionType : u8
    {
        None = 0,
        Sgd = 1,
        Png = 2,
    };

    struct Region final
    {
        u16 x;
        u16 y;
        u16 width;
        u16 height;
    };
}

static const bstr magic = "MGD "_b;

static void decompress_sgd_alpha(
    const bstr &input, io::BaseByteStream &output_stream)
{
    io::MemoryStream input_stream(input);
    while (!input_stream.eof())
    {
        auto flag = input_stream.read_le<u16>();
        if (flag & 0x8000)
        {
            const u8 alpha = input_stream.read<u8>();
            for (auto i : algo::range((flag & 0x7FFF) + 1))
            {
                output_stream.skip(3);
                output_stream.write<u8>(alpha ^ 0xFF);
            }
        }
        else
        {
            while (flag-- && !input_stream.eof())
            {
                u8 alpha = input_stream.read<u8>();
                output_stream.skip(3);
                output_stream.write<u8>(alpha ^ 0xFF);
            }
        }
    }
    output_stream.seek(0);
}

static void decompress_sgd_bgr_strategy_1(
    io::BaseByteStream &input_stream,
    io::BaseByteStream &output_stream,
    const u8 flag)
{
    output_stream.skip(-4);
    u8 b = output_stream.read<u8>();
    u8 g = output_stream.read<u8>();
    u8 r = output_stream.read<u8>();
    output_stream.skip(1);
    for (const auto i : algo::range(flag & 0x3F))
    {
        const u16 delta = input_stream.read_le<u16>();
        if (delta & 0x8000)
        {
            b += delta & 0x1F;
            g += (delta >> 5) & 0x1F;
            r += (delta >> 10) & 0x1F;
        }
        else
        {
            b += ( delta        & 0xF) * (delta &   0x10 ? -1 : 1);
            g += ((delta >>  5) & 0xF) * (delta &  0x200 ? -1 : 1);
            r += ((delta >> 10) & 0xF) * (delta & 0x4000 ? -1 : 1);
        }

        output_stream.write<u8>(b);
        output_stream.write<u8>(g);
        output_stream.write<u8>(r);
        output_stream.skip(1);
    }
}

static void decompress_sgd_bgr_strategy_2(
    io::BaseByteStream &input_stream,
    io::BaseByteStream &output_stream,
    const u8 flag)
{
    const u8 b = input_stream.read<u8>();
    const u8 g = input_stream.read<u8>();
    const u8 r = input_stream.read<u8>();
    for (const auto i : algo::range((flag & 0x3F) + 1))
    {
        output_stream.write<u8>(b);
        output_stream.write<u8>(g);
        output_stream.write<u8>(r);
        output_stream.skip(1);
    }
}

static void decompress_sgd_bgr_strategy_3(
    io::BaseByteStream &input_stream,
    io::BaseByteStream &output_stream,
    const u8 flag)
{
    for (const auto i : algo::range(flag))
    {
        output_stream.write(input_stream.read(3));
        output_stream.skip(1);
    }
}

static void decompress_sgd_bgr(
    const bstr &input, io::BaseByteStream &output_stream)
{
    std::function<void(io::BaseByteStream &, io::BaseByteStream &, u8)> func;
    io::MemoryStream input_stream(input);
    while (!input_stream.eof())
    {
        u8 flag = input_stream.read<u8>();
        switch (flag & 0xC0)
        {
            case 0x80: func = decompress_sgd_bgr_strategy_1; break;
            case 0x40: func = decompress_sgd_bgr_strategy_2; break;
            case 0x00: func = decompress_sgd_bgr_strategy_3; break;
            default: throw err::CorruptDataError("Bad decompression flag");
        }
        func(input_stream, output_stream, flag);
    }
    output_stream.seek(0);
}

static bstr decompress_sgd(const bstr &input, size_t output_size)
{
    bstr output(output_size);
    io::MemoryStream output_stream(output);

    io::MemoryStream tmp_stream(input);

    auto alpha_size = tmp_stream.read_le<u32>();
    auto alpha_data = tmp_stream.read(alpha_size);
    decompress_sgd_alpha(alpha_data, output_stream);

    auto color_size = tmp_stream.read_le<u32>();
    auto color_data = tmp_stream.read(color_size);
    decompress_sgd_bgr(color_data, output_stream);

    return output_stream.read_to_eof();
}

static std::vector<std::unique_ptr<Region>> read_region_data(
    io::BaseByteStream &input_stream)
{
    std::vector<std::unique_ptr<Region>> regions;
    while (input_stream.tell() < input_stream.size())
    {
        input_stream.skip(4);
        size_t regions_size = input_stream.read_le<u32>();
        size_t region_count = input_stream.read_le<u16>();
        size_t meta_format = input_stream.read_le<u16>();
        size_t bytes_left = input_stream.size() - input_stream.tell();
        if (meta_format != 4)
            throw err::NotSupportedError("Unexpected meta format");
        if (regions_size != bytes_left)
            throw err::CorruptDataError("Region size mismatch");

        for (auto i : algo::range(region_count))
        {
            auto region = std::make_unique<Region>();
            region->x = input_stream.read_le<u16>();
            region->y = input_stream.read_le<u16>();
            region->width = input_stream.read_le<u16>();
            region->height = input_stream.read_le<u16>();
            regions.push_back(std::move(region));
        }

        if (input_stream.tell() + 4 >= input_stream.size())
            break;
        input_stream.skip(4);
    }
    return regions;
}

static res::Image read_image(
    const Logger &logger,
    const bstr &input,
    CompressionType compression_type,
    size_t size_original,
    size_t width,
    size_t height)
{
    if (compression_type == CompressionType::None)
        return res::Image(width, height, input, res::PixelFormat::BGRA8888);

    if (compression_type == CompressionType::Sgd)
    {
        const auto data = decompress_sgd(input, size_original);
        return res::Image(width, height, data, res::PixelFormat::BGRA8888);
    }

    if (compression_type == CompressionType::Png)
    {
        io::File png_file;
        png_file.stream.write(input);
        const auto png_decoder = dec::png::PngImageDecoder();
        return png_decoder.decode(logger, png_file);
    }

    throw err::NotSupportedError("Unsupported compression type");
}

bool MgdImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image MgdImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    u16 data_offset = input_file.stream.read_le<u16>();
    u16 format = input_file.stream.read_le<u16>();
    input_file.stream.skip(4);
    u16 width = input_file.stream.read_le<u16>();
    u16 height = input_file.stream.read_le<u16>();
    u32 size_original = input_file.stream.read_le<u32>();
    u32 size_compressed_total = input_file.stream.read_le<u32>();
    const auto compression_type
        = static_cast<const CompressionType>(input_file.stream.read_le<u32>());
    input_file.stream.skip(64);

    const size_t size_compressed = input_file.stream.read_le<u32>();
    if (size_compressed_total != size_compressed + 4)
        throw err::CorruptDataError("Compressed data size mismatch");

    auto image = read_image(
        logger,
        input_file.stream.read(size_compressed),
        compression_type,
        size_original,
        width,
        height);
    read_region_data(input_file.stream);
    return image;
}

static auto _ = dec::register_decoder<MgdImageDecoder>("nsystem/mgd");
