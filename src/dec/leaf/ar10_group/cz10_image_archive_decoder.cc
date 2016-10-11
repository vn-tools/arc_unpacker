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

#include "dec/leaf/ar10_group/cz10_image_archive_decoder.h"
#include <array>
#include "algo/format.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "cz10"_b;

namespace
{
    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        size_t width, height, channels;
    };
}

static bstr decompress(
    const bstr &input,
    const size_t width,
    const size_t height,
    const size_t channels)
{
    const auto stride = width * channels;

    bstr last_line(stride);
    bstr output;
    output.reserve(stride * height);

    io::MemoryByteStream input_stream(input);
    for (const auto y : algo::range(height))
    {
        const auto control = input_stream.read<u8>();
        bstr line;

        if (control == 0)
        {
            line = input_stream.read(stride);
        }
        else if (control == 1)
        {
            const auto chunk_size = input_stream.read_be<u16>();
            const auto chunk = input_stream.read(chunk_size);
            line = algo::pack::zlib_inflate(chunk);
        }
        else if (control == 2)
        {
            const auto chunk_size = input_stream.read_be<u16>();
            const auto chunk = input_stream.read(chunk_size);
            line = algo::pack::zlib_inflate(chunk);
            for (const auto i : algo::range(1, stride))
                line[i] = line[i - 1] - line[i];
        }
        else if (control == 3)
        {
            const auto chunk_size = input_stream.read_be<u16>();
            const auto chunk = input_stream.read(chunk_size);
            line = algo::pack::zlib_inflate(chunk);
            for (const auto i : algo::range(stride))
                line[i] = last_line[i] - line[i];
        }
        else
            throw err::CorruptDataError("Unexpected control byte");

        output += line;
        last_line = line;
    }

    return output;
}

bool Cz10ImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Cz10ImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();

    input_file.stream.seek(magic.size());
    const auto image_count = input_file.stream.read_le<u32>();
    auto current_offset = input_file.stream.pos() + image_count * 16;
    for (const auto i : algo::range(image_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->width = input_file.stream.read_le<u16>();
        entry->height = input_file.stream.read_le<u16>();
        input_file.stream.skip(4);
        entry->size = input_file.stream.read_le<u32>();
        entry->channels = input_file.stream.read_le<u32>();
        entry->offset = current_offset;
        current_offset += entry->size;
        meta->entries.push_back(std::move(entry));
    }

    return meta;
}

std::unique_ptr<io::File> Cz10ImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    const auto data = decompress(
        input_file.stream.seek(entry->offset).read(entry->size),
        entry->width,
        entry->height,
        entry->channels);

    if (entry->channels != 4)
        throw err::UnsupportedChannelCountError(entry->channels);

    res::Image image(entry->width, entry->height);
    const auto *data_ptr = data.get<const u8>();
    for (const auto y : algo::range(entry->height))
    for (const auto c : algo::range(entry->channels))
    for (const auto x : algo::range(entry->width))
    {
        image.at(x, y)[c] = *data_ptr++;
    }
    const auto encoder = enc::png::PngImageEncoder();
    return encoder.encode(logger, image, entry->path);
}

algo::NamingStrategy Cz10ImageArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Sibling;
}

static auto _ = dec::register_decoder<Cz10ImageArchiveDecoder>("leaf/cz10");
