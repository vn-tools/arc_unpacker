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

#include "dec/eagls/gr_image_decoder.h"
#include "algo/crypt/lcg.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "dec/microsoft/bmp_image_decoder.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::eagls;

static const bstr key = "EAGLS_SYSTEM"_b;
static const u32 xor_value = 0x75BD924;

static size_t guess_output_size(const bstr &data)
{
    io::MemoryByteStream bmp_stream(algo::pack::lzss_decompress(data, 30));
    bmp_stream.skip(10);
    const auto pixels_start = bmp_stream.read_le<u32>();
    bmp_stream.skip(4);
    const auto width = bmp_stream.read_le<u32>();
    const auto height = bmp_stream.read_le<u32>();
    bmp_stream.skip(2);
    const auto depth = bmp_stream.read_le<u16>();
    const auto stride = ((width + 3) / 4) * 4;
    return pixels_start + stride * height * (depth >> 3);
}

bool GrImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("gr");
}

res::Image GrImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    // According to Crass the offset, key and LCG kind vary for other games.

    auto data = input_file.stream.read(input_file.stream.size() - 1);
    const auto seed = input_file.stream.read<u8>() ^ xor_value;

    algo::crypt::Lcg lcg(algo::crypt::LcgKind::ParkMillerRevised, seed);
    for (const auto i : algo::range(0, std::min<size_t>(0x174B, data.size())))
        data[i] ^= key[lcg.next() % key.size()];

    const auto output_size = guess_output_size(data);
    data = algo::pack::lzss_decompress(data, output_size);

    const auto bmp_file_decoder = dec::microsoft::BmpImageDecoder();
    io::File bmp_file(input_file.path, data);
    return bmp_file_decoder.decode(logger, bmp_file);
}

static auto _ = dec::register_decoder<GrImageDecoder>("eagls/gr");
