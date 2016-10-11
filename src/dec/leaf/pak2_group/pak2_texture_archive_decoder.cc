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

#include "dec/leaf/pak2_group/pak2_texture_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "\x88\x33\x67\x82"_b;
static const bstr canvas_magic1 = "\x70\x2B\xCD\xC8"_b;
static const bstr canvas_magic2 = "\x03\xC5\x0D\xA6"_b;

namespace
{
    struct Chunk final
    {
        uoff_t offset;
        size_t size;
        int x, y;
        size_t width, height;
    };

    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        std::vector<Chunk> chunks;
    };
}

bool Pak2TextureArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(4).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Pak2TextureArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();

    input_file.stream.seek(24);
    const auto image_count = input_file.stream.read_le<u16>();
    const auto chunk_count = input_file.stream.read_le<u16>();
    input_file.stream.skip(4);
    auto last_chunk_offset = input_file.stream.read_le<u16>();

    const auto data_offset = input_file.stream.pos()
        + image_count * 2
        + chunk_count * 36
        + 36;

    std::vector<size_t> image_chunk_counts;
    for (const auto i : algo::range(image_count))
    {
        const auto chunk_offset = input_file.stream.read_le<u16>();
        image_chunk_counts.push_back(chunk_offset - last_chunk_offset);
        last_chunk_offset = chunk_offset;
    }

    int i =0;
    for (const auto image_chunk_count : image_chunk_counts)
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        for (const auto j : algo::range(image_chunk_count))
        {
            Chunk chunk;
            input_file.stream.skip(8);
            chunk.x = static_cast<s16>(input_file.stream.read_le<u16>());
            chunk.y = static_cast<s16>(input_file.stream.read_le<u16>());
            input_file.stream.skip(4);
            chunk.width = input_file.stream.read_le<u16>();
            chunk.height = input_file.stream.read_le<u16>();
            input_file.stream.skip(4);
            chunk.offset = input_file.stream.read_le<u32>() + data_offset;
            input_file.stream.skip(8);
            entry->chunks.push_back(chunk);
        }
        if (!entry->chunks.empty())
            meta->entries.push_back(std::move(entry));
    }

    return meta;
}

std::unique_ptr<io::File> Pak2TextureArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    int min_x = 0, min_y = 0, max_x = 0, max_y = 0;
    for (const auto &chunk : entry->chunks)
    {
        min_x = std::min<int>(min_x, chunk.x);
        min_y = std::min<int>(min_y, chunk.y);
        max_x = std::max<int>(max_x, chunk.x + chunk.width);
        max_y = std::max<int>(max_y, chunk.y + chunk.height);
    }
    const auto width = max_x - min_x;
    const auto height = max_y - min_y;
    res::Image image(width, height);
    for (auto &c : image)
    {
        c.r = c.g = c.b = 0;
        c.a = 0xFF;
    }
    for (const auto &chunk : entry->chunks)
    {
        input_file.stream.seek(chunk.offset);
        auto chunk_image = res::Image(
            chunk.width,
            chunk.height,
            input_file.stream.read(chunk.width * chunk.height * 2),
            res::PixelFormat::BGRnA5551);
        chunk_image.flip_vertically();
        image.overlay(
            chunk_image,
            chunk.x,
            chunk.y,
            res::Image::OverlayKind::OverwriteAll);
    }
    const auto encoder = enc::png::PngImageEncoder();
    return encoder.encode(logger, image, entry->path);
}

algo::NamingStrategy Pak2TextureArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Sibling;
}

static auto _ = dec::register_decoder<Pak2TextureArchiveDecoder>(
    "leaf/pak2-texture");
