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

#include "dec/shiina_rio/s25_image_archive_decoder.h"
#include <map>
#include "algo/format.h"
#include "algo/range.h"
#include "enc/png/png_image_encoder.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::shiina_rio;

static const bstr magic = "S25\x00"_b;

namespace
{
    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        size_t width, height;
        uoff_t offset;
        u32 flags;
    };
}

static bstr read_row(
    io::BaseByteStream &input_stream, const uoff_t row_offset)
{
    input_stream.seek(row_offset);
    auto row_size = input_stream.read_le<u16>();
    if (row_offset & 1)
    {
        input_stream.skip(1);
        row_size--;
    }
    return input_stream.read(row_size);
}

static bstr decode_row(
    const bstr &input,
    const CustomArchiveEntry &entry,
    const size_t repeat)
{
    bstr output(entry.width * 4);
    auto output_ptr = output.get<u8>();
    const auto output_end = output.end<const u8>();

    io::MemoryByteStream input_stream(input);
    auto left = entry.width;
    while (output_ptr < output_end)
    {
        if (input_stream.pos() & 1)
            input_stream.skip(1);

        const u16 tmp = input_stream.read_le<u16>();

        const size_t method = tmp >> 13;
        const size_t skip = (tmp >> 11) & 3;
        input_stream.skip(skip);
        size_t count = tmp & 0x7FF;
        if (!count)
            count = input_stream.read_le<u32>();
        if (count > left)
            count = left;
        left -= count;

        if (method == 2)
        {
            if (input_stream.left() < count * 3)
                count = input_stream.left() / 3;
            auto chunk = input_stream.read(3 * count);
            auto chunk_ptr = chunk.get<const u8>();
            for (const auto j : algo::range(repeat))
            for (const auto i : algo::range(3, count * 3))
                chunk[i] += chunk[i - 3];
            for (const auto i : algo::range(count))
            {
                *output_ptr++ = *chunk_ptr++;
                *output_ptr++ = *chunk_ptr++;
                *output_ptr++ = *chunk_ptr++;
                *output_ptr++ = 0xFF;
            }
        }

        else if (method == 3)
        {
            const auto chunk = input_stream.read(3);
            for (const auto i : algo::range(count))
            {
                *output_ptr++ = chunk[0];
                *output_ptr++ = chunk[1];
                *output_ptr++ = chunk[2];
                *output_ptr++ = 0xFF;
            }
        }

        else if (method == 4)
        {
            if (input_stream.left() < count * 4)
                count = input_stream.left() / 4;
            auto chunk = input_stream.read(4 * count);
            auto chunk_ptr = chunk.get<const u8>();
            for (const auto j : algo::range(repeat))
            for (const auto i : algo::range(4, count * 4))
                chunk[i] += chunk[i - 4];
            for (const auto i : algo::range(count))
            {
                *output_ptr++ = chunk_ptr[1];
                *output_ptr++ = chunk_ptr[2];
                *output_ptr++ = chunk_ptr[3];
                *output_ptr++ = chunk_ptr[0];
                chunk_ptr += 4;
            }
        }

        else if (method == 5)
        {
            const auto chunk = input_stream.read(4);
            for (const auto i : algo::range(count))
            {
                *output_ptr++ = chunk[1];
                *output_ptr++ = chunk[2];
                *output_ptr++ = chunk[3];
                *output_ptr++ = chunk[0];
            }
        }

        else
            output_ptr += count * 4;
    }

    return output;
}

static bstr read_plain(
    io::BaseByteStream &input_stream, const CustomArchiveEntry &entry)
{
    bstr data;
    data.reserve(entry.width * entry.height * 4);
    std::vector<uoff_t> row_offsets(entry.height);
    for (auto &offset : row_offsets)
        offset = input_stream.read_le<u32>();
    for (const auto y : algo::range(entry.height))
    {
        const auto row_offset = row_offsets[y];
        const auto input_row = read_row(input_stream, row_offset);
        const auto output_row = decode_row(input_row, entry, 0);
        data += output_row;
    }
    return data;
}

static bstr read_incremental(
    io::BaseByteStream &input_stream,
    const CustomArchiveEntry &entry,
    const dec::ArchiveMeta &meta)
{
    std::vector<uoff_t> row_offsets(entry.height);
    for (auto &row_offset : row_offsets)
        row_offset = input_stream.read_le<u32>();

    std::map<uoff_t, int> rows_count;
    for (const auto &e : meta.entries)
    {
        const auto other_entry
            = static_cast<const CustomArchiveEntry*>(e.get());
        input_stream.peek(other_entry->offset, [&]()
        {
            for (const auto y : algo::range(other_entry->height))
            {
                const auto row_offset = input_stream.read_le<u32>();
                rows_count[row_offset]++;
            }
        });
    }

    bstr data;
    data.reserve(entry.width * entry.height * 4);
    for (const auto y : algo::range(entry.height))
    {
        const auto row_offset = row_offsets[y];
        const auto input_row = read_row(input_stream, row_offset);
        const auto output_row = decode_row(
            input_row, entry, rows_count[row_offset]);
        data += output_row;
    }
    return data;
}

bool S25ImageArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> S25ImageArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    std::vector<uoff_t> offsets;
    for (const auto i : algo::range(file_count))
    {
        const auto offset = input_file.stream.read_le<u32>();
        if (offset)
            offsets.push_back(offset);
    }

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto offset : offsets)
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        input_file.stream.seek(offset);
        entry->width = input_file.stream.read_le<u32>();
        entry->height = input_file.stream.read_le<u32>();
        input_file.stream.skip(8);
        entry->flags = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.pos();
        if (!entry->width || !entry->height)
            continue;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> S25ImageArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);
    const auto pixel_data = entry->flags & 0x80000000
        ? read_incremental(input_file.stream, *entry, m)
        : read_plain(input_file.stream, *entry);
    const auto encoder = enc::png::PngImageEncoder();
    return encoder.encode(
        logger,
        res::Image(
            entry->width,
            entry->height,
            pixel_data,
            res::PixelFormat::BGRA8888),
        entry->path);
}

algo::NamingStrategy S25ImageArchiveDecoder::naming_strategy() const
{
    return algo::NamingStrategy::Sibling;
}

static auto _ = dec::register_decoder<S25ImageArchiveDecoder>("shiina-rio/s25");
