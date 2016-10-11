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

#include "dec/silky/akb_image_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "err.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::silky;

static const bstr magic1 = "AKB "_b;
static const bstr magic2 = "AKB+"_b;

bool AkbImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic1.size()) == magic1
        || input_file.stream.seek(0).read(magic2.size()) == magic2;
}

res::Image AkbImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto plus = input_file.stream.seek(0).read(magic2.size()) == magic2;
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();
    const auto channels = input_file.stream.read_be<u32>() & 0x40 ? 3 : 4;
    const auto background
        = res::read_pixel<res::PixelFormat::BGRA8888>(input_file.stream);
    const auto x1 = input_file.stream.read_le<s32>();
    const auto y1 = input_file.stream.read_le<s32>();
    const auto x2 = input_file.stream.read_le<s32>();
    const auto y2 = input_file.stream.read_le<s32>();
    if (y2 < y1 || x2 < x1)
        throw err::BadDataSizeError();

    std::unique_ptr<res::Image> base_image;
    if (plus)
    {
        const auto base_name = input_file.stream.read_to_zero(32).str();

        auto base_file = VirtualFileSystem::get_by_stem(
            io::path(base_name).stem());
        if (base_file && is_recognized(*base_file))
        {
            base_image = std::make_unique<res::Image>(
                decode(logger, *base_file));
        }
    }

    if (!base_image)
    {
        base_image = std::make_unique<res::Image>(width, height);
        for (auto &c : *base_image)
            c = background;
    }

    const auto canvas_width = x2 - x1;
    const auto canvas_height = y2 - y1;
    const auto canvas_stride = canvas_width * channels;
    auto data = algo::pack::lzss_decompress(
        input_file.stream.read_to_eof(), canvas_stride * canvas_height);

    for (const auto y1 : algo::range(canvas_height / 2))
    {
        const auto y2 = canvas_height - 1 - y1;
        auto source_ptr = &data[y1 * canvas_stride];
        auto target_ptr = &data[y2 * canvas_stride];
        for (const auto x : algo::range(canvas_stride))
            std::swap(source_ptr[x], target_ptr[x]);
    }

    for (const auto x : algo::range(channels, canvas_stride))
        data[x] += data[x - channels];
    for (const auto y : algo::range(1, canvas_height))
    {
        auto source_ptr = &data[(y - 1) * canvas_stride];
        auto target_ptr = &data[y * canvas_stride];
        for (const auto x : algo::range(canvas_stride))
            target_ptr[x] += source_ptr[x];
    }

    const auto fmt = channels == 4
        ? res::PixelFormat::BGRA8888
        : res::PixelFormat::BGR888;
    res::Image overlay(x2 - x1, y2 - y1, data, fmt);

    if (plus)
        for (auto &c : overlay)
            if (c.r == 0 && c.g == 0xFF && c.b == 0)
                c.a = 0;

    base_image->overlay(
        overlay, x1, y1, res::Image::OverlayKind::OverwriteNonTransparent);
    return *base_image;
}

static auto _ = dec::register_decoder<AkbImageDecoder>("silky/akb");
