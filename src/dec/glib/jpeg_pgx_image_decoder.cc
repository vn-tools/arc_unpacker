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

#include "dec/glib/jpeg_pgx_image_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "dec/glib/custom_lzss.h"
#include "io/memory_byte_stream.h"

// This is a bit different from plain PGX - namely, it involves two LZSS passes.

using namespace au;
using namespace au::dec::glib;

static const bstr magic = "PGX\x00"_b;

static bstr extract_pgx_stream(const bstr &jpeg_data)
{
    bstr output;
    output.reserve(jpeg_data.size());
    io::MemoryByteStream jpeg_stream(jpeg_data);
    jpeg_stream.skip(2); // soi
    jpeg_stream.skip(2); // header chunk
    jpeg_stream.skip(jpeg_stream.read_be<u16>() - 2);
    while (jpeg_stream.read_be<u16>() == 0xFFE3)
        output += jpeg_stream.read(jpeg_stream.read_be<u16>() - 2);
    return output;
}

bool JpegPgxImageDecoder::is_recognized_impl(io::File &input_file) const
{
    u16 marker = input_file.stream.read_be<u16>();
    // soi
    if (marker != 0xFFD8)
        return false;
    marker = input_file.stream.read_be<u16>();
    // header chunk
    if (marker != 0xFFE0)
        return false;
    input_file.stream.skip(input_file.stream.read_be<u16>() - 2);
    // PGX start
    marker = input_file.stream.read_be<u16>();
    if (marker != 0xFFE3)
        return false;
    if (input_file.stream.read_be<u16>() < magic.size())
        return false;
    return input_file.stream.read(magic.size()) == magic;
}

res::Image JpegPgxImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto pgx_data = extract_pgx_stream(input_file.stream.read_to_eof());
    io::MemoryByteStream pgx_stream(pgx_data);

    pgx_stream.skip(magic.size());
    pgx_stream.skip(4);
    const auto width = pgx_stream.read_le<u32>();
    const auto height = pgx_stream.read_le<u32>();
    const auto transparent = pgx_stream.read_le<u16>() != 0;
    pgx_stream.skip(2);
    const auto source_size = pgx_stream.read_le<u32>();
    const auto target_size = width * height * 4;
    pgx_stream.skip(8);

    if (!transparent)
    {
        pgx_stream.skip(8);
        const auto tmp1 = pgx_stream.read_le<u32>();
        const auto tmp2 = pgx_stream.read_le<u32>();
        const auto extra_size = (tmp2 & 0x00FF00FF) | (tmp1 & 0xFF00FF00);
        // why?
        custom_lzss_decompress(pgx_stream, extra_size);
    }

    const auto target = custom_lzss_decompress(
        pgx_stream.read_to_eof(), target_size);

    res::Image image(width, height, target, res::PixelFormat::BGRA8888);
    if (!transparent)
        for (auto &c : image)
            c.a = 0xFF;
    return image;
}

static auto _ = dec::register_decoder<JpegPgxImageDecoder>("glib/jpeg-pgx");
