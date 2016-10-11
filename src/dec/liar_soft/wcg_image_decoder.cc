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

#include "dec/liar_soft/wcg_image_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "dec/liar_soft/cg_decompress.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::liar_soft;

static const bstr magic = "WG"_b;

bool WcgImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic.size()) != magic)
        return false;

    const int version = input_file.stream.read_le<u16>();
    if (((version & 0xF) != 1) || ((version & 0x1C0) != 64))
        return false;

    return true;
}

res::Image WcgImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());

    input_file.stream.skip(2);
    const auto depth = input_file.stream.read_le<u16>();
    if (depth != 32)
        throw err::UnsupportedBitDepthError(depth);
    input_file.stream.skip(2);

    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    const auto canvas_size = width * height;

    bstr output(canvas_size * 4);
    cg_decompress(output, 2, 4, input_file.stream, 2);
    cg_decompress(output, 0, 4, input_file.stream, 2);

    for (const auto i : algo::range(0, output.size(), 4))
        output[i + 3] ^= 0xFF;

    return res::Image(width, height, output, res::PixelFormat::BGRA8888);
}

static auto _ = dec::register_decoder<WcgImageDecoder>("liar-soft/wcg");
