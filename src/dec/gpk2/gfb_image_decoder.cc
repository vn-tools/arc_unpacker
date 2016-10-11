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

#include "dec/gpk2/gfb_image_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::gpk2;

static const bstr magic = "GFB\x20"_b;

bool GfbImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image GfbImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(12);
    const auto size_comp = input_file.stream.read_le<u32>();
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto data_offset = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    input_file.stream.skip(2);
    const auto depth = input_file.stream.read_le<u16>();

    // found no examples for this
    // std::unique_ptr<res::Palette> palette;
    // if (depth == 8 || data_offset != 0x40)
    // {
    //     input_file.stream.seek(0x40);
    //     palette = std::make_unique<res::Palette>(
    //         std::min<size_t>((data_offset - 0x40) / 4, 256),
    //         input_file.stream,
    //         res::PixelFormat::BGR888X);
    // }

    input_file.stream.seek(data_offset);
    bstr data;
    if (size_comp)
    {
        data = algo::pack::lzss_decompress(
            input_file.stream.read(size_comp), size_orig);
    }
    else
    {
        data = input_file.stream.read(size_orig);
    }

    std::unique_ptr<res::Image> image;
    if (depth == 32)
    {
        auto has_alpha = false;
        for (const auto i : algo::range(3, data.size(), 4))
            if (data[i] != 0)
                has_alpha = true;
        image = std::make_unique<res::Image>(
            width,
            height,
            data,
            has_alpha
                ? res::PixelFormat::BGRA8888
                : res::PixelFormat::BGR888X);
    }
    else if (depth == 24)
    {
        image = std::make_unique<res::Image>(
            width, height, data, res::PixelFormat::BGR888);
    }
    // else if (depth == 8)
    // {
    //     image = palette
    //         ? std::make_unique<res::Image>(width, height, data, *palette)
    //         : std::make_unique<res::Image>(
    //             width, height, data, res::PixelFormat::Gray8);
    // }
    else
    {
        throw err::UnsupportedBitDepthError(depth);
    }

    image->flip_vertically();
    return *image;
}

static auto _ = dec::register_decoder<GfbImageDecoder>("gpk2/gfb");
