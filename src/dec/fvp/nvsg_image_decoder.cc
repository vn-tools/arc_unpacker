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

#include "dec/fvp/nvsg_image_decoder.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::fvp;

static const bstr hzc1_magic = "hzc1"_b;
static const bstr nvsg_magic = "NVSG"_b;

bool NvsgImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(hzc1_magic.size()) != hzc1_magic)
        return false;
    input_file.stream.skip(8);
    return input_file.stream.read(nvsg_magic.size()) == nvsg_magic;
}

res::Image NvsgImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(hzc1_magic.size());
    const auto size_orig = input_file.stream.read_le<u32>();
    input_file.stream.skip(4); // nvsg header size
    input_file.stream.skip(nvsg_magic.size());
    input_file.stream.skip(2);
    const auto format = input_file.stream.read_le<u16>();
    const auto width = input_file.stream.read_le<u16>();
    auto height = input_file.stream.read_le<u16>();
    input_file.stream.skip(8);
    const auto image_count = input_file.stream.read_le<u32>();
    input_file.stream.skip(8);

    bstr data = algo::pack::zlib_inflate(input_file.stream.read_to_eof());

    res::PixelFormat pixel_format;
    switch (format)
    {
        case 0:
            pixel_format = res::PixelFormat::BGR888;
            break;

        case 1:
            pixel_format = res::PixelFormat::BGRA8888;
            break;

        case 2:
            height *= image_count;
            pixel_format = res::PixelFormat::BGRA8888;
            break;

        case 3:
            pixel_format = res::PixelFormat::Gray8;
            break;

        case 4:
            for (const auto i : algo::range(data.size()))
                if (data.get<u8>()[i])
                    data.get<u8>()[i] = 255;
            pixel_format = res::PixelFormat::Gray8;
            break;

        default:
            throw err::NotSupportedError("Unexpected pixel format");
    }

    return res::Image(width, height, data, pixel_format);
}

static auto _ = dec::register_decoder<NvsgImageDecoder>("fvp/nvsg");
