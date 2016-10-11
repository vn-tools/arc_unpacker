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

#include "dec/playstation/gtf_image_decoder.h"
#include "algo/range.h"
#include "dec/microsoft/dxt/dxt_decoders.h"
#include "err.h"

using namespace au;
using namespace au::dec::playstation;
using namespace au::dec::microsoft::dxt;

static const bstr magic = "\x01\x05\x00\x00"_b;

namespace
{
    enum class CellGcmTextureType : u8
    {
        B8                 = 0x81,
        A1R5G5B5           = 0x82,
        A4R4G4B4           = 0x83,
        R5G6B5             = 0x84,
        A8R8G8B8           = 0x85,
        CompressedDxt1     = 0x86,
        CompressedDxt23    = 0x87,
        CompressedDxt45    = 0x88,
        G8B8               = 0x8B,
        CompressedB8R8G8R8 = 0x8D,
        CompressedR8B8R8G8 = 0x8E,
        R6G5B5             = 0x8F,
        Depth24D8          = 0x90,
        Depth24D8Float     = 0x91,
        Depth16            = 0x92,
        Depth16Float       = 0x93,
        X16                = 0x94,
        Y16X16             = 0x95,
        R5G5B5A1           = 0x97,
        CompressedHilo8    = 0x98,
        CompressedHiloS8   = 0x99,
        W16Z16Y16X16Float  = 0x9A,
        W32Z32Y32X32Float  = 0x9B,
        X32Float           = 0x9C,
        D1R5G5B5           = 0x9D,
        D8R8G8B8           = 0x9E,
        Y16X16Float        = 0x9F,
    };

    struct GtfHeader final
    {
        u32 version;
        size_t content_size;
        size_t file_count;
    };

    struct GtfSpec final
    {
        size_t header_size;
        u32 mipmap, dimensions, cube, flags;
        size_t width, height, depth;
        uoff_t offset;
    };
}

static GtfHeader read_header(io::BaseByteStream &input_stream)
{
    GtfHeader header;
    header.version = input_stream.read_be<u32>();
    header.content_size = input_stream.read_be<u32>();
    header.file_count = input_stream.read_be<u32>();
    return header;
}

static GtfSpec read_spec(io::BaseByteStream &input_stream)
{
    GtfSpec spec;
    input_stream.skip(4);
    spec.header_size = input_stream.read_be<u32>();
    spec.offset = input_stream.pos() + spec.header_size - 4;
    input_stream.skip(4);
    spec.flags = input_stream.read<u8>();
    spec.mipmap = input_stream.read<u8>();
    spec.dimensions = input_stream.read<u8>();
    spec.cube = input_stream.read<u8>();
    input_stream.skip(4);
    spec.width = input_stream.read_be<u16>();
    spec.height = input_stream.read_be<u16>();
    spec.depth = input_stream.read_be<u16>();
    input_stream.skip(2);
    input_stream.skip(4);
    return spec;
}

bool GtfImageDecoder::is_recognized_impl(io::File &input_file) const
{
    const auto file_size = input_file.stream.size();
    const auto content_size = input_file.stream.seek(4).read_be<u32>();
    const auto header_size = input_file.stream.seek(16).read_be<u32>();
    return header_size + content_size == file_size;
}

res::Image GtfImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto header = read_header(input_file.stream);
    if (header.file_count != 1)
        throw err::NotSupportedError("Multi textures are not supported");

    std::vector<GtfSpec> specs;
    for (const auto i : algo::range(header.file_count))
        specs.push_back(read_spec(input_file.stream));

    const auto &spec = specs.at(0);
    if (spec.depth != 1)
        throw err::NotSupportedError("3D textures are not supported");

    input_file.stream.seek(spec.header_size);

    const auto format = static_cast<CellGcmTextureType>(spec.flags & 0x9F);
    if (format == CellGcmTextureType::CompressedDxt1)
        return *decode_dxt1(input_file.stream, spec.width, spec.height);

    if (format == CellGcmTextureType::CompressedDxt23)
        return *decode_dxt3(input_file.stream, spec.width, spec.height);

    if (format == CellGcmTextureType::CompressedDxt45)
        return *decode_dxt5(input_file.stream, spec.width, spec.height);

    throw err::NotSupportedError("Only DXT-packed textures are supported");
}

static auto _ = dec::register_decoder<GtfImageDecoder>("playstation/gtf");
