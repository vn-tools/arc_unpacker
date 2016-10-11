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

#include "dec/leaf/leafpack_group/lc3_image_decoder.h"
#include "dec/leaf/common/custom_lzss.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "LEAFC64\x00"_b;

bool Lc3ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image Lc3ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    input_file.stream.skip(4);
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();

    const auto alpha_pos = input_file.stream.read_le<u32>();
    const auto color_pos = input_file.stream.read_le<u32>();

    input_file.stream.seek(color_pos);
    const auto color_data = common::custom_lzss_decompress(
        input_file.stream.read_to_eof(), width * height * 2);
    res::Image image(width, height, color_data, res::PixelFormat::BGR555X);

    input_file.stream.seek(alpha_pos);
    auto mask_data = common::custom_lzss_decompress(
        input_file.stream.read(color_pos - alpha_pos), width * height);
    for (auto &c : mask_data)
        c <<= 3;
    res::Image mask(width, height, mask_data, res::PixelFormat::Gray8);

    image.apply_mask(mask);
    image.flip_vertically();
    return image;
}

static auto _ = dec::register_decoder<Lc3ImageDecoder>("leaf/lc3");
