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

#include "dec/kaguya/an00_image_archive_decoder.h"
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"

using namespace au;
using namespace au::dec::kaguya;

static const bstr magic = "AN00"_b;

namespace
{
    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        uoff_t offset;
        size_t x, y;
        size_t width, height;
    };
}

algo::NamingStrategy An00ImageArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Sibling;
}

bool An00ImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> An00ImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto base_x = input_file.stream.read_le<u32>();
    const auto base_y = input_file.stream.read_le<u32>();
    const auto base_width = input_file.stream.read_le<u32>();
    const auto base_height = input_file.stream.read_le<u32>();
    const auto unk_count = input_file.stream.read_le<u16>();
    input_file.stream.skip(2);
    input_file.stream.skip(unk_count * 4);
    auto meta = std::make_unique<dec::ArchiveMeta>();
    const auto file_count = input_file.stream.read_le<u16>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->x = input_file.stream.read_le<u32>();
        entry->y = input_file.stream.read_le<u32>();
        entry->width = input_file.stream.read_le<u32>();
        entry->height = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.pos();
        input_file.stream.skip(4 * entry->width * entry->height);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> An00ImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    res::Image image(
        entry->width,
        entry->height,
        input_file.stream.seek(entry->offset),
        res::PixelFormat::BGRA8888);
    image.flip_vertically();
    return enc::png::PngImageEncoder().encode(logger, image, entry->path);
}

static auto _ = dec::register_decoder<An00ImageArchiveDecoder>("kaguya/an00");
