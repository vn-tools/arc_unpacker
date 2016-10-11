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

#include "dec/leaf/kcap_group/bbm_image_decoder.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::leaf;

bool BbmImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("bbm");
}

res::Image BbmImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto total_width = input_file.stream.read_le<u16>();
    const auto total_height = input_file.stream.read_le<u16>();

    const auto chunk_width = input_file.stream.read_le<u16>();
    const auto chunk_height = input_file.stream.read_le<u16>();
    const auto chunk_count_x = input_file.stream.read_le<u16>();
    const auto chunk_count_y = input_file.stream.read_le<u16>();
    const auto chunk_size = input_file.stream.read_le<u32>();

    res::Image image(total_width, total_height);
    for (const auto chunk_y : algo::range(chunk_count_y))
    for (const auto chunk_x : algo::range(chunk_count_x))
    {
        io::MemoryByteStream chunk_stream(input_file.stream.read(chunk_size));
        chunk_stream.skip(5);
        const auto color_count = chunk_stream.read_le<u16>();
        chunk_stream.skip(11);
        const res::Palette palette(
            color_count, chunk_stream, res::PixelFormat::BGRA8888);
        const res::Image sub_image(
            chunk_width, chunk_height, chunk_stream, palette);
        const auto base_x = chunk_x * chunk_width;
        const auto base_y = chunk_y * chunk_height;
        for (const auto y : algo::range(chunk_height))
        for (const auto x : algo::range(chunk_width))
        {
            image.at(base_x + x, base_y + y) = sub_image.at(x, y);
        }
    }
    return image;
}

static auto _ = dec::register_decoder<BbmImageDecoder>("leaf/bbm");
