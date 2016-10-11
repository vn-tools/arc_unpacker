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

#include "dec/microsoft/dds_image_decoder.h"
#include "algo/endian.h"
#include "algo/format.h"
#include "algo/range.h"
#include "dec/microsoft/dxt/dxt_decoders.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::microsoft;
using namespace au::dec::microsoft::dxt;

namespace
{
    enum class D3d10ResourceDimension : u32
    {
        Unknown    = 0,
        Buffer     = 1,
        Texture1D  = 2,
        Texture2D  = 3,
        Texture3D  = 4,
    };

    enum DdsPixelFormatFlags
    {
        DDPF_ALPHAPIXELS = 0x1,
        DDPF_ALPHA = 0x2,
        DDPF_FOURCC = 0x4,
        DDPF_RGB = 0x40,
        DDPF_YUV = 0x200,
        DDPF_LUMINACE = 0x20000,
    };

    struct DdsPixelFormat final
    {
        u32 size;
        DdsPixelFormatFlags flags;
        bstr four_cc;
        u32 rgb_bit_count;
        u32 r_bit_mask;
        u32 g_bit_mask;
        u32 b_bit_mask;
        u32 a_bit_mask;
    };

    struct DdsHeaderDx10 final
    {
        u32 dxgi_format;
        D3d10ResourceDimension resource_dimension;
        u32 misc_flag;
        u32 array_size;
        u32 misc_flags2;
    };

    enum DdsHeaderFlags
    {
        DDSD_CAPS        = 0x1,
        DDSD_HEIGHT      = 0x2,
        DDSD_WIDTH       = 0x4,
        DDSD_PITCH       = 0x8,
        DDSD_PIXELFORMAT = 0x1000,
        DDSD_MIPMAPCOUNT = 0x20000,
        DDSD_LINEARSIZE  = 0x80000,
        DDSD_DEPTH       = 0x800000,
    };

    struct DdsHeader final
    {
        u32 size;
        DdsHeaderFlags flags;
        u32 height;
        u32 width;
        u32 pitch_or_linear_size;
        u32 depth;
        u32 mip_map_count;
        DdsPixelFormat pixel_format;
        u32 caps[4];
    };
}

static const bstr magic = "DDS\x20"_b;
static const bstr magic_dxt1 = "DXT1"_b;
static const bstr magic_dxt2 = "DXT2"_b;
static const bstr magic_dxt3 = "DXT3"_b;
static const bstr magic_dxt4 = "DXT4"_b;
static const bstr magic_dxt5 = "DXT5"_b;
static const bstr magic_dx10 = "DX10"_b;

static void fill_pixel_format(
    io::BaseByteStream &input_stream, DdsPixelFormat &pixel_format)
{
    pixel_format.size = input_stream.read_le<u32>();
    pixel_format.flags
        = static_cast<DdsPixelFormatFlags>(input_stream.read_le<u32>());
    pixel_format.four_cc = input_stream.read(4);
    pixel_format.rgb_bit_count = input_stream.read_le<u32>();
    pixel_format.r_bit_mask = input_stream.read_le<u32>();
    pixel_format.g_bit_mask = input_stream.read_le<u32>();
    pixel_format.b_bit_mask = input_stream.read_le<u32>();
    pixel_format.a_bit_mask = input_stream.read_le<u32>();
}

static std::unique_ptr<DdsHeader> read_header(io::BaseByteStream &input_stream)
{
    auto header = std::make_unique<DdsHeader>();
    header->size = input_stream.read_le<u32>();
    header->flags = static_cast<DdsHeaderFlags>(input_stream.read_le<u32>());
    header->height = input_stream.read_le<u32>();
    header->width = input_stream.read_le<u32>();
    header->pitch_or_linear_size = input_stream.read_le<u32>();
    header->depth = input_stream.read_le<u32>();
    header->mip_map_count = input_stream.read_le<u32>();
    input_stream.skip(4 * 11);
    fill_pixel_format(input_stream, header->pixel_format);
    for (const auto i : algo::range(4))
        header->caps[i] = input_stream.read_le<u32>();
    input_stream.skip(4);
    return header;
}

static std::unique_ptr<DdsHeaderDx10> read_header_dx10(
    io::BaseByteStream &input_stream)
{
    auto header = std::make_unique<DdsHeaderDx10>();
    header->dxgi_format = input_stream.read_le<u32>();
    header->resource_dimension
        = static_cast<D3d10ResourceDimension>(input_stream.read_le<u32>());
    header->misc_flag = input_stream.read_le<u32>();
    header->array_size = input_stream.read_le<u32>();
    header->misc_flags2 = input_stream.read_le<u32>();
    return header;
}

bool DdsImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image DdsImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    auto header = read_header(input_file.stream);
    if (header->pixel_format.four_cc == magic_dx10)
        read_header_dx10(input_file.stream);

    const auto width = header->width;
    const auto height = header->height;

    std::unique_ptr<res::Image> image(nullptr);
    if (header->pixel_format.flags & DDPF_FOURCC)
    {
        if (header->pixel_format.four_cc == magic_dxt1)
            image = decode_dxt1(input_file.stream, width, height);
        else if (header->pixel_format.four_cc == magic_dxt3)
            image = decode_dxt3(input_file.stream, width, height);
        else if (header->pixel_format.four_cc == magic_dxt5)
            image = decode_dxt5(input_file.stream, width, height);
        else
        {
            throw err::NotSupportedError(algo::format(
                "%s textures are not supported",
                header->pixel_format.four_cc.c_str()));
        }
    }
    else if (header->pixel_format.flags & DDPF_RGB)
    {
        if (header->pixel_format.rgb_bit_count == 32)
        {
            image.reset(new res::Image(
                width, height, input_file.stream, res::PixelFormat::BGRA8888));
        }
    }

    if (image == nullptr)
        throw err::NotSupportedError("Unsupported pixel format");

    return *image;
}

static auto _ = dec::register_decoder<DdsImageDecoder>("microsoft/dds");
