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

#include "dec/leaf/pak1_group/grp_image_decoder.h"
#include "algo/range.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::leaf;

static int detect_version(io::File &input_file)
{
    int version = 1;
    input_file.stream.seek(0);
    size_t width = input_file.stream.read_le<u16>();
    size_t height = input_file.stream.read_le<u16>();
    if (!width && !height)
    {
        width = input_file.stream.read_le<u16>();
        height = input_file.stream.read_le<u16>();
        version = 2;
    }
    if (width * height + version * 4 == input_file.stream.size())
        return version;
    return -1;
}

static res::Image decode_image(io::File &input_file)
{
    const auto version = detect_version(input_file);
    input_file.stream.seek(version == 1 ? 0 : 4);
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();
    const auto data = input_file.stream.read(width * height);
    return res::Image(width, height, data, res::PixelFormat::Gray8);
}

static res::Palette decode_palette(io::File &input_file)
{
    input_file.stream.seek(0);
    const auto count = input_file.stream.read_le<u16>();
    auto palette = res::Palette(256);
    for (const auto i : algo::range(count))
    {
        auto index = input_file.stream.read<u8>();
        palette[index].a = 0xFF;
        palette[index].b = input_file.stream.read<u8>();
        palette[index].g = input_file.stream.read<u8>();
        palette[index].r = input_file.stream.read<u8>();
    }
    return palette;
}

bool GrpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return detect_version(input_file) > 0;
}

res::Image GrpImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    auto image = decode_image(input_file);
    const auto mask_file = VirtualFileSystem::get_by_name(
        io::path(input_file.path).change_extension("msk").name());
    const auto palette_file = VirtualFileSystem::get_by_name(
        io::path(input_file.path).change_extension("c16").name());

    if (palette_file)
    {
        const auto palette = decode_palette(*palette_file);
        image.apply_palette(palette);
    }
    if (mask_file)
    {
        mask_file->stream.seek(0);
        auto mask_data = mask_file->stream.read_to_eof();
        for (auto &c : mask_data)
            c ^= 0xFF;
        image.apply_mask(res::Image(
            image.width(), image.height(), mask_data, res::PixelFormat::Gray8));
    }
    return image;
}

static auto _ = dec::register_decoder<GrpImageDecoder>("leaf/grp");
