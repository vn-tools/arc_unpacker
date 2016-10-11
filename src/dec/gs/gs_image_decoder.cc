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

#include "dec/gs/gs_image_decoder.h"
#include "algo/pack/lzss.h"
#include "err.h"

using namespace au;
using namespace au::dec::gs;

static const bstr magic = "\x00\x00\x04\x00"_b;

bool GsImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image GsImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    const auto size_comp = input_file.stream.read_le<u32>();
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto header_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    const auto depth = input_file.stream.read_le<u32>();
    const bool use_transparency = input_file.stream.read_le<u32>() > 0;

    input_file.stream.seek(header_size);
    auto data = input_file.stream.read(size_comp);
    data = algo::pack::lzss_decompress(data, size_orig);

    if (depth == 8)
    {
        res::Palette palette(256, data, res::PixelFormat::BGRA8888);
        for (auto &c : palette)
            c.a = 0xFF;
        return res::Image(width, height, data.substr(256 * 4), palette);
    }

    if (depth == 32)
    {
        res::Image image(width, height, data, res::PixelFormat::BGRA8888);
        if (!use_transparency)
            for (auto &c : image)
                c.a = 0xFF;
        return image;
    }

    throw err::UnsupportedBitDepthError(depth);
}

static auto _ = dec::register_decoder<GsImageDecoder>("gs/gfx");
