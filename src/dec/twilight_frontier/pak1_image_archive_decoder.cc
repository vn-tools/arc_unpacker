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

#include "dec/twilight_frontier/pak1_image_archive_decoder.h"
#include "algo/format.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::twilight_frontier;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        std::vector<res::Palette> palettes;
    };

    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        size_t width, height;
        size_t depth;
    };
}

bool Pak1ImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("dat"))
        return false;
    const auto palette_count = input_file.stream.read<u8>();
    input_file.stream.skip(palette_count * 512);
    while (input_file.stream.left())
    {
        input_file.stream.skip(4 * 3);
        input_file.stream.skip(1);
        input_file.stream.skip(input_file.stream.read_le<u32>());
    }
    return input_file.stream.left() == 0;
}

std::unique_ptr<dec::ArchiveMeta> Pak1ImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<CustomArchiveMeta>();
    const auto palette_count = input_file.stream.read<u8>();
    for (const auto i : algo::range(palette_count))
    {
        meta->palettes.push_back(res::Palette(
            256, input_file.stream.read(512), res::PixelFormat::BGRA5551));
    }
    meta->palettes.push_back(res::Palette(256));

    size_t i = 0;
    while (input_file.stream.left())
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->width = input_file.stream.read_le<u32>();
        entry->height = input_file.stream.read_le<u32>();
        input_file.stream.skip(4);
        entry->depth = input_file.stream.read<u8>();
        entry->size = input_file.stream.read_le<u32>();
        entry->path = algo::format("%04d", i++);
        entry->offset = input_file.stream.pos();
        input_file.stream.skip(entry->size);
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> Pak1ImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);

    auto chunk_size = 0;
    if (entry->depth == 32 || entry->depth == 24)
        chunk_size = 4;
    else if (entry->depth == 16)
        chunk_size = 2;
    else if (entry->depth == 8)
        chunk_size = 1;
    else
        throw err::UnsupportedBitDepthError(entry->depth);

    bstr output(entry->width * entry->height * chunk_size);
    auto output_ptr = algo::make_ptr(output);
    input_file.stream.seek(entry->offset);
    io::MemoryByteStream input_stream(input_file.stream, entry->size);

    while (output_ptr.left() && input_stream.left())
    {
        size_t repeat;
        if (entry->depth == 32 || entry->depth == 24)
            repeat = input_stream.read_le<u32>();
        else if (entry->depth == 16)
            repeat = input_stream.read_le<u16>();
        else if (entry->depth == 8)
            repeat = input_stream.read<u8>();
        else
            throw err::UnsupportedBitDepthError(entry->depth);

        auto chunk = input_stream.read(chunk_size);
        while (repeat-- && output_ptr.left())
            for (auto &c : chunk)
                *output_ptr++ = c;
    }

    std::unique_ptr<res::Image> image;
    if (entry->depth == 32)
    {
        image = std::make_unique<res::Image>(
            entry->width, entry->height, output, res::PixelFormat::BGRA8888);
    }
    else if (entry->depth == 24)
    {
        image = std::make_unique<res::Image>(
            entry->width, entry->height, output, res::PixelFormat::BGR888X);
    }
    else if (entry->depth == 16)
    {
        image = std::make_unique<res::Image>(
            entry->width, entry->height, output, res::PixelFormat::BGRA5551);
    }
    else if (entry->depth == 8)
    {
        image = std::make_unique<res::Image>(
            entry->width, entry->height, output, meta->palettes[0]);
    }
    else
        throw err::UnsupportedBitDepthError(entry->depth);

    const auto encoder = enc::png::PngImageEncoder();
    return encoder.encode(logger, *image, entry->path);
}

static auto _ = dec::register_decoder<Pak1ImageArchiveDecoder>(
    "twilight-frontier/pak1-gfx");
