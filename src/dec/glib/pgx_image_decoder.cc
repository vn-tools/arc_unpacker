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

#include "dec/glib/pgx_image_decoder.h"
#include "algo/range.h"
#include "dec/glib/custom_lzss.h"

using namespace au;
using namespace au::dec::glib;

static const bstr magic = "PGX\x00"_b;

bool PgxImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image PgxImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size() + 4);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    const auto transparent = input_file.stream.read_le<u16>() != 0;
    input_file.stream.skip(2);
    const auto source_size = input_file.stream.read_le<u32>();
    const auto target_size = width * height * 4;

    input_file.stream.seek(input_file.stream.size() - source_size);
    const auto source = input_file.stream.read(source_size);
    const auto target = custom_lzss_decompress(source, target_size);
    res::Image image(width, height, target, res::PixelFormat::BGRA8888);
    if (!transparent)
        for (auto &c : image)
            c.a = 0xFF;
    return image;
}

static auto _ = dec::register_decoder<PgxImageDecoder>("glib/pgx");
