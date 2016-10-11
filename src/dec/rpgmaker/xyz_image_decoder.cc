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

#include "dec/rpgmaker/xyz_image_decoder.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::rpgmaker;

static const bstr magic = "XYZ1"_b;

bool XyzImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image XyzImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();
    const auto data = algo::pack::zlib_inflate(input_file.stream.read_to_eof());
    io::MemoryByteStream data_stream(data);
    const auto pal_data = data_stream.read(256 * 3);
    const auto pix_data = data_stream.read_to_eof();
    res::Palette palette(256, pal_data, res::PixelFormat::RGB888);
    return res::Image(width, height, pix_data, palette);
}

static auto _ = dec::register_decoder<XyzImageDecoder>("rpgmaker/xyz");
