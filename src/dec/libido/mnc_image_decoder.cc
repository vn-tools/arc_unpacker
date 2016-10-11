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

#include "dec/libido/mnc_image_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::libido;

static const bstr magic = "\x48\x48\x36\x10\x0E\x00\x00\x00\x00\x00"_b;

bool MncImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image MncImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    const auto offset_to_pixels = input_file.stream.read_le<u32>();
    const auto header_size = input_file.stream.read_le<u32>();
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    input_file.stream.skip(2);
    const auto bit_depth = input_file.stream.read_le<u16>();
    input_file.stream.skip(4);
    const auto data_size = input_file.stream.read_le<u32>();
    input_file.stream.seek(offset_to_pixels);
    const auto data = input_file.stream.read(data_size);

    if (bit_depth != 24)
        throw err::UnsupportedBitDepthError(bit_depth);

    res::Image image(width, height, data, res::PixelFormat::BGR888);
    image.flip_vertically();
    return image;
}

static auto _ = dec::register_decoder<MncImageDecoder>("libido/mnc");
