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

#include "dec/yuris/ycg_image_decoder.h"
#include "algo/pack/zlib.h"
#include "err.h"

using namespace au;
using namespace au::dec::yuris;

static const bstr magic = "YCG\x00"_b;

bool YcgImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image YcgImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto width              = input_file.stream.read_le<u32>();
    const auto height             = input_file.stream.read_le<u32>();
    const auto depth              = input_file.stream.read_le<u32>();
    const auto compression_method = input_file.stream.read_le<u32>();
    input_file.stream.skip(12);
    const auto size_orig1         = input_file.stream.read_le<u32>();
    const auto size_comp1         = input_file.stream.read_le<u32>();
    input_file.stream.skip(8);
    const auto size_orig2         = input_file.stream.read_le<u32>();
    const auto size_comp2         = input_file.stream.read_le<u32>();

    bstr data;
    if (compression_method == 1)
    {
        input_file.stream.seek(0x38);
        data += algo::pack::zlib_inflate(input_file.stream.read(size_comp1));
        data += algo::pack::zlib_inflate(input_file.stream.read(size_comp2));
    }
    else
    {
        throw err::NotSupportedError("Unknown compression method");
    }

    if (depth != 32)
        throw err::UnsupportedBitDepthError(depth);

    return res::Image(width, height, data, res::PixelFormat::BGRA8888);
}

static auto _ = dec::register_decoder<YcgImageDecoder>("yuris/ycg");
