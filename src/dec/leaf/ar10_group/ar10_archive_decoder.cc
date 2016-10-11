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

#include "dec/leaf/ar10_group/ar10_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "ar10"_b;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        u8 archive_key;
    };
}

bool Ar10ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Ar10ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    const auto offset_to_data = input_file.stream.read_le<u32>();
    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->archive_key = input_file.stream.read<u8>();
    PlainArchiveEntry *last_entry = nullptr;
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = algo::sjis_to_utf8(
            input_file.stream.read_to_zero()).str();
        entry->offset = input_file.stream.read_le<u32>() + offset_to_data;
        if (last_entry)
            last_entry->size = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }
    if (last_entry)
        last_entry->size = input_file.stream.size() - last_entry->offset;
    return std::move(meta);
}

std::unique_ptr<io::File> Ar10ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);

    input_file.stream.seek(entry->offset);
    const auto data_size = input_file.stream.read_le<u32>();
    const auto key_size = input_file.stream.read<u8>() ^ meta->archive_key;
    const auto key = input_file.stream.read(key_size);
    auto data = input_file.stream.read(data_size);

    for (const auto i : algo::range(data.size()))
        data[i] ^= key[i % key.size()];

    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> Ar10ArchiveDecoder::get_linked_formats() const
{
    return {"leaf/cz10"};
}

static auto _ = dec::register_decoder<Ar10ArchiveDecoder>("leaf/ar10");
