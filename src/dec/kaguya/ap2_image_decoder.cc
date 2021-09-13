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

#include "dec/kaguya/ap2_image_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::kaguya;

static const bstr magic = "AP-2"_b;

bool Ap2ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image Ap2ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto x_offset = input_file.stream.read_le<u32>();
    const auto y_offset = input_file.stream.read_le<u32>();
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    const auto depth = input_file.stream.read_le<u32>();
    if (depth != 24 && depth != 32)
        throw err::UnsupportedBitDepthError(depth);
    const auto data = input_file.stream.read_to_eof();
    return res::Image(width, height, data, res::PixelFormat::BGRA8888)
        .flip_vertically()
        .offset(x_offset, y_offset);
}

static auto _ = dec::register_decoder<Ap2ImageDecoder>("kaguya/ap2");
