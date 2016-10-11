// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "dec/nsystem/mgd_image_decoder.h"
#include "algo/range.h"
#include "dec/png/png_image_decoder.h"
#include "err.h"
#include "io/memory_byte_stream.h"

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
    io::MemoryByteStream input_stream(input);
    while (input_stream.left())
    {
        auto flag = input_stream.read_le<u16>();
        if (flag & 0x8000)
        {
            const u8 alpha = input_stream.read<u8>();
            for (const auto i : algo::range((flag & 0x7FFF) + 1))
            {
                output_stream.skip(3);
                output_stream.write<u8>(alpha ^ 0xFF);
            }
        }
        else
        {
            while (flag-- && input_stream.left())
            {
                const auto alpha = input_stream.read<u8>();
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
    auto b = output_stream.read<u8>();
    auto g = output_stream.read<u8>();
    auto r = output_stream.read<u8>();
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
    io::MemoryByteStream input_stream(input);
    while (input_stream.left())
    {
        auto flag = input_stream.read<u8>();
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
    io::MemoryByteStream output_stream(output);

    io::MemoryByteStream tmp_stream(input);

    const auto alpha_size = tmp_stream.read_le<u32>();
    const auto alpha_data = tmp_stream.read(alpha_size);
    decompress_sgd_alpha(alpha_data, output_stream);

    const auto color_size = tmp_stream.read_le<u32>();
    const auto color_data = tmp_stream.read(color_size);
    decompress_sgd_bgr(color_data, output_stream);

    return output_stream.read_to_eof();
}

static std::vector<std::unique_ptr<Region>> read_region_data(
    io::BaseByteStream &input_stream)
{
    std::vector<std::unique_ptr<Region>> regions;
    while (input_stream.left())
    {
        input_stream.skip(4);
        const auto regions_size = input_stream.read_le<u32>();
        const auto region_count = input_stream.read_le<u16>();
        const auto meta_format = input_stream.read_le<u16>();
        const auto bytes_left = input_stream.size() - input_stream.pos();
        if (meta_format != 4)
            throw err::NotSupportedError("Unexpected meta format");
        if (regions_size != bytes_left)
            throw err::CorruptDataError("Region size mismatch");

        for (const auto i : algo::range(region_count))
        {
            auto region = std::make_unique<Region>();
            region->x = input_stream.read_le<u16>();
            region->y = input_stream.read_le<u16>();
            region->width = input_stream.read_le<u16>();
            region->height = input_stream.read_le<u16>();
            regions.push_back(std::move(region));
        }

        if (input_stream.left() < 4)
            break;
        input_stream.skip(4);
    }
    return regions;
}

static res::Image read_image(
    const Logger &logger,
    const bstr &input,
    CompressionType compression_type,
    size_t size_orig,
    size_t width,
    size_t height)
{
    if (compression_type == CompressionType::None)
        return res::Image(width, height, input, res::PixelFormat::BGRA8888);

    if (compression_type == CompressionType::Sgd)
    {
        const auto data = decompress_sgd(input, size_orig);
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

    const auto data_offset = input_file.stream.read_le<u16>();
    const auto format = input_file.stream.read_le<u16>();
    input_file.stream.skip(4);
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto size_comp_total = input_file.stream.read_le<u32>();
    const auto compression_type
        = static_cast<const CompressionType>(input_file.stream.read_le<u32>());
    input_file.stream.skip(64);

    const auto size_comp = input_file.stream.read_le<u32>();
    if (size_comp_total != size_comp + 4)
        throw err::CorruptDataError("Compressed data size mismatch");

    auto image = read_image(
        logger,
        input_file.stream.read(size_comp),
        compression_type,
        size_orig,
        width,
        height);
    read_region_data(input_file.stream);
    return image;
}

static auto _ = dec::register_decoder<MgdImageDecoder>("nsystem/mgd");
