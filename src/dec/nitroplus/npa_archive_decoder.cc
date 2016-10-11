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

#include "dec/nitroplus/npa_archive_decoder.h"
#include "algo/locale.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::nitroplus;

static const bstr magic = "NPA\x01\x00\x00\x00"_b;

namespace
{
    enum class EntryType : u8
    {
        Directory = 1,
        File = 2
    };

    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        std::shared_ptr<NpaPlugin> plugin;
        u32 key1, key2;
        bool files_are_encrypted;
        bool files_are_compressed;
    };

    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        bstr path_orig;
    };

    struct TableEntry final
    {
        std::string name;
    };
}

static void decrypt_file_name(
    const CustomArchiveMeta &meta, bstr &name, size_t file_index)
{
    u32 tmp = meta.plugin->file_name_key(meta.key1, meta.key2);
    for (const auto char_pos : algo::range(name.size()))
    {
        u32 key = 0xFC * char_pos;
        key -= tmp >> 0x18;
        key -= tmp >> 0x10;
        key -= tmp >> 0x08;
        key -= tmp & 0xFF;
        key -= file_index >> 0x18;
        key -= file_index >> 0x10;
        key -= file_index >> 0x08;
        key -= file_index;
        name[char_pos] += (key & 0xFF);
    }
}

static void decrypt_file_data(
    const CustomArchiveMeta &meta, const CustomArchiveEntry &entry, bstr &data)
{
    u32 key = meta.plugin->data_key;
    for (const auto i : algo::range(entry.path_orig.size()))
        key -= entry.path_orig[i];
    key *= entry.path_orig.size();
    key += meta.key1 * meta.key2;
    key *= entry.size_orig;
    key &= 0xFF;

    u8 *data_ptr = data.get<u8>();
    const auto size = 0x1000 + entry.path_orig.size();
    for (const auto i : algo::range(std::min(size, data.size())))
        data_ptr[i] = meta.plugin->permutation[data_ptr[i]] - key - i;
}

bool NpaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> NpaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->plugin = plugin_manager.get();

    input_file.stream.seek(magic.size());
    meta->key1 = input_file.stream.read_le<u32>();
    meta->key2 = input_file.stream.read_le<u32>();
    meta->files_are_compressed = input_file.stream.read<u8>() > 0;
    meta->files_are_encrypted = input_file.stream.read<u8>() > 0;
    const auto total_entry_count = input_file.stream.read_le<u32>();
    const auto folder_count = input_file.stream.read_le<u32>();
    const auto file_count = input_file.stream.read_le<u32>();
    input_file.stream.skip(8);
    const auto table_size = input_file.stream.read_le<u32>();
    const auto data_offset = input_file.stream.pos() + table_size;

    for (const auto i : algo::range(total_entry_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();

        const auto name_size = input_file.stream.read_le<u32>();
        entry->path_orig = input_file.stream.read(name_size);
        decrypt_file_name(*meta, entry->path_orig, i);
        entry->path = algo::sjis_to_utf8(entry->path_orig).str();

        const auto entry_type = input_file.stream.read<EntryType>();
        input_file.stream.skip(4);

        entry->offset = input_file.stream.read_le<u32>() + data_offset;
        entry->size_comp = input_file.stream.read_le<u32>();
        entry->size_orig = input_file.stream.read_le<u32>();

        if (entry_type == EntryType::Directory)
            continue;
        if (entry_type != EntryType::File)
            throw err::NotSupportedError("Unknown file type");
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> NpaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);

    if (meta->files_are_encrypted)
        decrypt_file_data(*meta, *entry, data);

    if (meta->files_are_compressed)
        data = algo::pack::zlib_inflate(data);

    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<NpaArchiveDecoder>("nitroplus/npa");
