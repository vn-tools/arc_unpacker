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

#include "dec/malie/libp_archive_decoder.h"
#include "algo/range.h"
#include "dec/malie/common/camellia_stream.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::malie;

static const auto magic = "LIBP"_b;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        common::LibPlugin plugin;
    };
}

static void read_dir(
    io::BaseByteStream &input_stream,
    const uoff_t base_offset,
    const std::vector<uoff_t> &offsets,
    CustomArchiveMeta &output_meta,

    const size_t base_index = 0,
    const size_t file_count = 1,
    const io::path root = "")
{
    for (const auto i : algo::range(file_count))
    {
        input_stream.seek((base_index + i) * 32);
        io::path path = root / input_stream.read(20).str(true);
        const auto flags = input_stream.read_le<u32>();
        const auto offset = input_stream.read_le<u32>();
        const auto size = input_stream.read_le<u32>();
        if (!(flags & 0x10000))
        {
            if (offset <= base_index)
                continue;
            read_dir(
                input_stream,
                base_offset,
                offsets,
                output_meta,
                offset,
                size,
                path);
        }
        else
        {
            auto entry = std::make_unique<dec::PlainArchiveEntry>();
            entry->path = path;
            entry->offset = base_offset + (offsets.at(offset) << 10);
            entry->size = size;
            output_meta.entries.push_back(std::move(entry));
        }
    }
}

bool LibpArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    for (const auto &plugin : plugin_manager.get_all())
    {
        common::CamelliaStream camellia_stream(input_file.stream, plugin.key);
        const auto maybe_magic = camellia_stream.seek(0).read(4);
        if (maybe_magic == magic)
            return true;
    }
    return false;
}

std::unique_ptr<dec::ArchiveMeta> LibpArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<CustomArchiveMeta>();

    const auto maybe_magic = input_file.stream.seek(0).read(magic.size());
    meta->plugin = maybe_magic == magic
        ? plugin_manager.get("noop")
        : plugin_manager.get();

    common::CamelliaStream camellia_stream(input_file.stream, meta->plugin.key);
    camellia_stream.seek(magic.size());
    const auto entry_count = camellia_stream.read_le<u32>();
    const auto offset_count = camellia_stream.read_le<u32>();
    camellia_stream.skip(4);

    io::MemoryByteStream table(camellia_stream.read(0x20 * entry_count));
    std::vector<uoff_t> offsets;
    for (const auto i : algo::range(offset_count))
        offsets.push_back(camellia_stream.read_le<u32>());
    const auto mask = meta->plugin.data_alignment - 1;
    const auto base_offset = (camellia_stream.pos() + mask) & ~mask;
    read_dir(table, base_offset, offsets, *meta);
    return std::move(meta);
}

std::unique_ptr<io::File> LibpArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    return std::make_unique<io::File>(
        entry->path,
        std::make_unique<common::CamelliaStream>(
            input_file.stream,
            meta->plugin.key,
            entry->offset,
            entry->size));
}

std::vector<std::string> LibpArchiveDecoder::get_linked_formats() const
{
    return {"malie/libp", "malie/mgf", "malie/dzi"};
}

static auto _ = dec::register_decoder<LibpArchiveDecoder>("malie/libp");
