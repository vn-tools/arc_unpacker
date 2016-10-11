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

#include "dec/playstation/gim_image_decoder.h"
#include <map>
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::playstation;

static const bstr magic = "MIG.00.1PSP\x00"_b;

namespace
{
    struct Chunk final
    {
        int type;
        uoff_t offset;
        size_t size;
    };
}

static std::unique_ptr<res::Palette> read_palette(
    io::BaseByteStream &input_stream, const Chunk &chunk)
{
    input_stream.seek(chunk.offset + 0x14);
    const auto format_id = input_stream.read_le<u16>();
    input_stream.skip(2);
    const auto color_count = input_stream.read_le<u16>();

    res::PixelFormat format;
    if      (format_id == 0) format = res::PixelFormat::RGB565;
    else if (format_id == 1) format = res::PixelFormat::RGBA5551;
    else if (format_id == 2) format = res::PixelFormat::RGBA4444;
    else if (format_id == 3) format = res::PixelFormat::RGBA8888;
    else throw err::NotSupportedError("Unknown palette format");

    input_stream.seek(chunk.offset + 0x50);
    return std::make_unique<res::Palette>(color_count, input_stream, format);
}

static bstr read_data(
    io::BaseByteStream &input_stream,
    const size_t width,
    const size_t height,
    const size_t bpp,
    const bool swizzled)
{
    const auto stride = width * bpp / 8;
    const auto block_stride = stride / 16;
    auto data = input_stream.read(height * stride);
    if (swizzled)
    {
        const bstr source = data;
        for (const auto y : algo::range(height))
        for (const auto x : algo::range(stride))
        {
            const auto block_x = x / 16;
            const auto block_y = y / 8;
            const auto block_idx = block_x + block_y * block_stride;
            const auto block_offset = block_idx * 16 * 8;
            const auto idx = block_offset
                + (x - block_x * 16)
                + ((y - block_y * 8) * 16);
            data[x + y * stride] = source.at(idx);
        }
    }
    return data;
}

static std::unique_ptr<res::Image> read_image(
    io::BaseByteStream &input_stream,
    const Chunk &chunk,
    std::unique_ptr<res::Palette> palette)
{
    input_stream.seek(chunk.offset + 0x14);
    const auto format_id = input_stream.read_le<u16>();
    const auto swizzled = input_stream.read_le<u16>() == 0x01;
    const auto width = (input_stream.read_le<u16>() + 15) & ~15;
    const auto height = (input_stream.read_le<u16>() + 7) & ~7;

    input_stream.seek(chunk.offset + 0x2C);
    const auto data_offset = input_stream.read_le<u32>();
    input_stream.seek(chunk.offset + 0x10 + data_offset);
    std::unique_ptr<res::Image> image;
    if (format_id < 4)
    {
        res::PixelFormat format;

        if      (format_id == 0) format = res::PixelFormat::RGB565;
        else if (format_id == 1) format = res::PixelFormat::RGBA5551;
        else if (format_id == 2) format = res::PixelFormat::RGBA4444;
        else if (format_id == 3) format = res::PixelFormat::RGBA8888;
        else throw err::NotSupportedError("Unknown pixel format");

        const auto data = read_data(
            input_stream,
            width,
            height,
            res::pixel_format_to_bpp(format) * 8,
            swizzled);
        image = std::make_unique<res::Image>(width, height, data, format);
    }
    else
    {
        size_t palette_bits;
        if      (format_id == 4) palette_bits = 4;
        else if (format_id == 5) palette_bits = 8;
        else if (format_id == 6) palette_bits = 16;
        else if (format_id == 7) palette_bits = 32;
        else throw err::NotSupportedError("Unknown pixel format");

        const auto data = read_data(
            input_stream, width, height, palette_bits, swizzled);
        if (palette_bits == 8)
            image = std::make_unique<res::Image>(width, height, data, *palette);
        else if (palette_bits == 4)
        {
            io::MemoryByteStream data_stream(data);
            image = std::make_unique<res::Image>(width, height);
            if (palette_bits == 4)
            {
                for (const auto y : algo::range(height))
                for (const auto x : algo::range(0, width, 2))
                {
                    const auto tmp = data_stream.read<u8>();
                    image->at(x + 0, y) = palette->at(tmp & 0xF);
                    image->at(x + 1, y) = palette->at(tmp >> 4);
                }
            }
            else if (palette_bits == 16)
            {
                for (const auto y : algo::range(height))
                for (const auto x : algo::range(width))
                    image->at(x, y) = palette->at(data_stream.read_le<u16>());
            }
            else if (palette_bits == 32)
            {
                for (const auto y : algo::range(height))
                for (const auto x : algo::range(width))
                    image->at(x, y) = palette->at(data_stream.read_le<u32>());
            }
        }
    }

    return image;
}

bool GimImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image GimImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0x30);
    std::map<int, Chunk> chunks;
    while (input_file.stream.left())
    {
        Chunk chunk;
        chunk.offset = input_file.stream.pos();
        chunk.type = input_file.stream.read_le<u32>();
        chunk.size = input_file.stream.read_le<u32>();
        if (input_file.stream.read_le<u32>() != chunk.size)
            throw err::NotSupportedError("Data is most probably compressed");
        input_file.stream.seek(chunk.offset + chunk.size);
        chunks[chunk.type] = chunk;
    }

    std::unique_ptr<res::Palette> palette;
    if (chunks.find(0x05) != chunks.end())
        palette = read_palette(input_file.stream, chunks[0x05]);

    if (chunks.find(0x04) == chunks.end())
        throw err::CorruptDataError("Missing bitmap");
    return *read_image(input_file.stream, chunks[0x04], std::move(palette));
}

static auto _ = dec::register_decoder<GimImageDecoder>("playstation/gim");
