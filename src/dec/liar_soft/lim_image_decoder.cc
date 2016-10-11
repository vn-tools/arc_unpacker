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

#include "dec/liar_soft/lim_image_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "dec/liar_soft/cg_decompress.h"
#include "err.h"

using namespace au;
using namespace au::dec::liar_soft;

static const bstr magic = "LM"_b;

bool LimImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    if (!(input_file.stream.read_le<u16>() & 0x10))
        return false;
    return true;
}

res::Image LimImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto version = input_file.stream.read_le<u16>();
    const auto depth = input_file.stream.read_le<u16>();
    const auto bits_per_channel = input_file.stream.read_le<u16>();
    if (bits_per_channel != 8)
        throw err::UnsupportedBitDepthError(bits_per_channel);

    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    const auto canvas_size = width * height;

    if (!(version & 0xF))
        throw err::UnsupportedVersionError(version);
    if (depth != 16)
        throw err::UnsupportedBitDepthError(depth);

    bstr output(canvas_size * 2);
    cg_decompress(output, 0, 2, input_file.stream, 2);
    res::Image image(width, height, output, res::PixelFormat::BGR565);

    if (input_file.stream.left())
    {
        output.resize(canvas_size);
        cg_decompress(output, 0, 1, input_file.stream, 1);
        for (auto &c : output)
            c ^= 0xFF;
        res::Image mask(width, height, output, res::PixelFormat::Gray8);
        image.apply_mask(mask);
    }

    return image;
}

static auto _ = dec::register_decoder<LimImageDecoder>("liar-soft/lim");
