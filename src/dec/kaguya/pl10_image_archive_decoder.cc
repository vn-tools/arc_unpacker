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

#include "dec/kaguya/pl10_image_archive_decoder.h"
#include "algo/range.h"
#include "dec/kaguya/common/rle.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::kaguya;

static const bstr magic = "PL10"_b;

namespace
{
    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        bstr data;
        size_t x, y;
        size_t width, height;
        size_t channels;
    };
}

algo::NamingStrategy Pl10ImageArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Sibling;
}

bool Pl10ImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Pl10ImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u16>();
    const auto x = input_file.stream.read_le<u32>();
    const auto y = input_file.stream.read_le<u32>();
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    auto meta = std::make_unique<dec::ArchiveMeta>();

    auto base_entry = std::make_unique<CustomArchiveEntry>();
    base_entry->x = input_file.stream.read_le<u32>();
    base_entry->y = input_file.stream.read_le<u32>();
    base_entry->width = input_file.stream.read_le<u32>();
    base_entry->height = input_file.stream.read_le<u32>();
    base_entry->channels = input_file.stream.read_le<u32>();
    base_entry->data = input_file.stream.read(
        base_entry->channels * base_entry->width * base_entry->height);
    auto last_entry = base_entry.get();
    meta->entries.push_back(std::move(base_entry));

    for (const auto i : algo::range(1, file_count))
    {
        const auto bands = input_file.stream.read<u8>();
        const auto size_comp = input_file.stream.read_le<u32>();
        const auto size_orig = last_entry->channels
            * (last_entry->x + last_entry->width)
            * (last_entry->y + last_entry->height);

        auto output = common::decompress_rle(
            input_file.stream.read(size_comp), size_orig, bands);

        for (const auto i : algo::range(output.size()))
            output[i] += last_entry->data[i];

        auto sub_entry = std::make_unique<CustomArchiveEntry>();
        sub_entry->x = last_entry->x;
        sub_entry->y = last_entry->y;
        sub_entry->width = last_entry->width;
        sub_entry->height = last_entry->height;
        sub_entry->channels = last_entry->channels;
        sub_entry->data = output;
        last_entry = sub_entry.get();
        meta->entries.push_back(std::move(sub_entry));
    }
    return meta;
}

std::unique_ptr<io::File> Pl10ImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    res::PixelFormat fmt;
    if (entry->channels == 3)
        fmt = res::PixelFormat::BGR888;
    else if (entry->channels == 4)
        fmt = res::PixelFormat::BGRA8888;
    else
        throw err::UnsupportedChannelCountError(entry->channels);
    res::Image image(entry->width, entry->height, entry->data, fmt);
    image.flip_vertically();
    return enc::png::PngImageEncoder().encode(logger, image, entry->path);
}

static auto _ = dec::register_decoder<Pl10ImageArchiveDecoder>("kaguya/pl10");
