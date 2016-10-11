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

#include "dec/cri/xtx_image_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "dec/microsoft/dxt/dxt_decoders.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::cri;

static const bstr magic = "xtx\x00"_b;

namespace
{
    struct Header final
    {
        size_t canvas_size() const
        {
            return width * height;
        }

        size_t aligned_canvas_size() const
        {
            return aligned_width * aligned_height;
        }

        u8 format;
        size_t aligned_width;
        size_t aligned_height;
        size_t width;
        size_t height;
        size_t offset_x;
        size_t offset_y;
    };
}

// wow
static size_t get_x(const int i, const size_t width, const u8 level)
{
    const int v1 = (level >> 2) + (level >> 1 >> (level >> 2));
    const int v2 = i << v1;
    const int v3 = (v2 & 0x3F) + ((v2 >> 2) & 0x1C0) + ((v2 >> 3) & 0x1FFFFE00);
    return ((((level << 3) - 1) & ((v3 >> 1) ^ ((v3 ^ (v3 >> 1)) & 0xF))) >> v1)
        + ((((((v2 >> 6) & 0xFF) + ((v3 >> (v1 + 5)) & 0xFE)) & 3)
            + (((v3 >> (v1 + 7)) % (((width + 31)) >> 5)) << 2)) << 3);
}

// wow
static size_t get_y(const int i, const size_t width, const u8 level)
{
    const int v1 = (level >> 2) + (level >> 1 >> (level >> 2));
    const int v2 = i << v1;
    const int v3 = (v2 & 0x3F) + ((v2 >> 2) & 0x1C0) + ((v2 >> 3) & 0x1FFFFE00);
    return ((v3 >> 4) & 1)
        + ((((v3 & ((level << 6) - 1) & -0x20)
            + ((((v2 & 0x3F)
                + ((v2 >> 2) & 0xC0)) & 0xF) << 1)) >> (v1 + 3)) & -2)
        + ((((v2 >> 10) & 2) + ((v3 >> (v1 + 6)) & 1)
            + (((v3 >> (v1 + 7)) / ((width + 31) >> 5)) << 2)) << 3);
}

static bstr read_tex_0(const Header &header, io::BaseByteStream &input_stream)
{
    const auto input = input_stream.read(header.aligned_canvas_size() * 4);
    bstr output(header.canvas_size() * 4);
    for (const auto i : algo::range(header.aligned_canvas_size()))
    {
        const auto x = get_x(i, header.aligned_width, 4);
        const auto y = get_y(i, header.aligned_width, 4);
        if (y >= header.height || x >= header.width)
            continue;
        const auto src = i * 4;
        const auto dst = (x + y * header.width) * 4;
        output[dst + 3] = input[src];
        output[dst + 2] = input[src + 1];
        output[dst + 1] = input[src + 2];
        output[dst]     = input[src + 3];
    }
    return output;
}

static bstr read_tex_1(const Header &header, io::BaseByteStream &input_stream)
{
    const auto input = input_stream.read(header.aligned_canvas_size() * 2);
    bstr output(header.canvas_size() * 2);
    for (const auto i : algo::range(header.aligned_canvas_size()))
    {
        const auto x = get_x(i, header.aligned_width, 2);
        const auto y = get_y(i, header.aligned_width, 2);
        if (y >= header.height || x >= header.width)
            continue;
        const auto src = i * 2;
        const auto dst = (x + y * header.width) * 2;
        output[dst + 1] = input[src];
        output[dst] = input[src + 1];
    }
    return output;
}

static bstr read_tex_2(const Header &header, io::BaseByteStream &input_stream)
{
    const auto texture_width = header.aligned_width >> 2;
    const auto texture_height = header.aligned_height >> 2;
    const auto input = input_stream.read(header.aligned_canvas_size());
    bstr output(header.aligned_canvas_size());
    int src = 0;
    for (const auto i : algo::range(texture_width * texture_height))
    {
        const auto x = get_x(i, texture_width, 16);
        const auto y = get_y(i, texture_width, 16);
        auto dst = (x + y * texture_width) * 16;
        for (const auto j : algo::range(8))
        {
            output[dst + 1] = input[src];
            output[dst] = input[src + 1];
            dst += 2;
            src += 2;
        }
    }

    io::MemoryByteStream tmp_stream(output);
    const auto image = dec::microsoft::dxt::decode_dxt5(
        tmp_stream, header.aligned_width, header.aligned_height);
    bstr new_output(header.width * header.height * 4);
    for (const auto y : algo::range(header.height))
    for (const auto x : algo::range(header.width))
    {
        const auto dst = (x + y * header.width) * 4;
        new_output[dst]     = image->at(x, y).b;
        new_output[dst + 1] = image->at(x, y).g;
        new_output[dst + 2] = image->at(x, y).r;
        new_output[dst + 3] = image->at(x, y).a;
    }
    return new_output;
}

static soff_t locate_start(io::BaseByteStream &input_stream)
{
    if (input_stream.seek(0).read(magic.size()) == magic)
        return 0;
    const auto header_size = input_stream.seek(0).read_le<u32>();
    if (header_size >= 0x1000)
        return -1;
    input_stream.seek(header_size);
    if (input_stream.read(magic.size()) == magic)
        return header_size;
    return -1;
}

bool XtxImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return locate_start(input_file.stream) != -1;
}

res::Image XtxImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(locate_start(input_file.stream) + magic.size());
    Header header;
    header.format = input_file.stream.read<u8>();
    input_file.stream.skip(3);
    header.aligned_width = input_file.stream.read_be<u32>();
    header.aligned_height = input_file.stream.read_be<u32>();
    header.width = input_file.stream.read_be<u32>();
    header.height = input_file.stream.read_be<u32>();
    header.offset_x = input_file.stream.read_be<u32>();
    header.offset_y = input_file.stream.read_be<u32>();

    if (header.format == 0)
    {
        return res::Image(
            header.width,
            header.height,
            read_tex_0(header, input_file.stream),
            res::PixelFormat::BGRA8888);
    }

    if (header.format == 1)
    {
        return res::Image(
            header.width,
            header.height,
            read_tex_1(header, input_file.stream),
            res::PixelFormat::BGR565);
    }

    if (header.format == 2)
    {
        return res::Image(
            header.width,
            header.height,
            read_tex_2(header, input_file.stream),
            res::PixelFormat::BGRA8888);
    }

    throw err::NotSupportedError("Unknown pixel format");
}

static auto _ = dec::register_decoder<XtxImageDecoder>("cri/xtx");
