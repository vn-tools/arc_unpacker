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

#include "dec/innocent_grey/iga_archive_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::innocent_grey;

static const bstr magic = "IGA0"_b;

namespace
{
    struct EntrySpec final
    {
        uoff_t name_offset; size_t name_size;
        uoff_t data_offset; size_t data_size;
    };
}

static u32 read_integer(io::BaseByteStream &input_stream)
{
    u32 ret = 0;
    while (!(ret & 1))
    {
        ret <<= 7;
        ret |= input_stream.read<u8>();
    }
    return ret >> 1;
}

bool IgaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> IgaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    input_file.stream.skip(12);

    const auto table_size = read_integer(input_file.stream);
    const auto table_start = input_file.stream.pos();
    const auto table_end = table_start + table_size;
    EntrySpec *last_entry_spec = nullptr;
    std::vector<std::unique_ptr<EntrySpec>> entry_specs;
    while (input_file.stream.pos() < table_end)
    {
        auto spec = std::make_unique<EntrySpec>();
        spec->name_offset = read_integer(input_file.stream);
        spec->data_offset = read_integer(input_file.stream);
        spec->data_size = read_integer(input_file.stream);
        if (last_entry_spec)
        {
            last_entry_spec->name_size
                = spec->name_offset - last_entry_spec->name_offset;
        }
        last_entry_spec = spec.get();
        entry_specs.push_back(std::move(spec));
    }
    if (!last_entry_spec)
        return std::make_unique<ArchiveMeta>();

    const auto names_size = read_integer(input_file.stream);
    const auto names_start = input_file.stream.pos();
    last_entry_spec->name_size = names_size - last_entry_spec->name_offset;

    const auto data_offset = input_file.stream.size()
        - last_entry_spec->data_offset
        - last_entry_spec->data_size;

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto &spec : entry_specs)
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        input_file.stream.seek(names_start + spec->name_offset);
        std::string name;
        for (const auto i : algo::range(spec->name_size))
            name += read_integer(input_file.stream);
        entry->path = name;
        entry->offset = data_offset + spec->data_offset;
        entry->size = spec->data_size;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> IgaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    for (const auto i : algo::range(data.size()))
        data[i] ^= (i + 2) & 0xFF;
    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<IgaArchiveDecoder>("innocent-grey/iga");
