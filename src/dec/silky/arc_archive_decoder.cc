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

#include "dec/silky/arc_archive_decoder.h"
#include "algo/locale.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::silky;

bool ArcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("arc"))
        return false;
    Logger dummy_logger;
    dummy_logger.mute();
    const auto meta = read_meta_impl(dummy_logger, input_file);
    if (!meta->entries.size())
        return false;
    const auto &last_entry
        = static_cast<const CompressedArchiveEntry&>(*meta->entries.back());
    return last_entry.offset + last_entry.size_comp == input_file.stream.size();
}

std::unique_ptr<dec::ArchiveMeta> ArcArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto table_size = input_file.stream.read_le<u32>();
    io::MemoryByteStream table_stream(input_file.stream.read(table_size));

    auto meta = std::make_unique<ArchiveMeta>();
    while (table_stream.left())
    {
        auto entry = std::make_unique<CompressedArchiveEntry>();
        const auto name_size = table_stream.read<u8>();
        auto name = table_stream.read(name_size);
        for (const auto j : algo::range(name.size()))
            name[j] += name.size() - j;
        entry->path = algo::sjis_to_utf8(name).str();
        entry->size_comp = table_stream.read_be<u32>();
        entry->size_orig = table_stream.read_be<u32>();
        entry->offset = table_stream.read_be<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> ArcArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CompressedArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);
    if (entry->size_comp != entry->size_orig)
        data = algo::pack::lzss_decompress(data, entry->size_orig);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> ArcArchiveDecoder::get_linked_formats() const
{
    return {"silky/akb"};
}

static auto _ = dec::register_decoder<ArcArchiveDecoder>("silky/arc");
