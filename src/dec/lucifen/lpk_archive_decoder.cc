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

#include "dec/lucifen/lpk_archive_decoder.h"
#include <stack>
#include "algo/binary.h"
#include "algo/locale.h"
#include "algo/pack/lzss.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "algo/str.h"
#include "io/memory_byte_stream.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::lucifen;

static const bstr magic = "LPK1"_b;

namespace
{
    enum LpkFlags
    {
        AlignedOffset = 0b0000'0001,
        Flag1         = 0b0000'0010,
        IsEncrypted1  = 0b0000'0100,
        IsCompressed  = 0b0000'1000,
        IsEncrypted2  = 0b0001'0000,
    };

    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        LpkPlugin plugin;
        LpkFlags flags;
        u32 content_key;
        bstr prefix;
    };

    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        u8 flags;
    };
}

static void decrypt_table(bstr &data, u32 key, u32 rotate_pattern)
{
    auto data_ptr = algo::make_ptr(data.get<u32>(), data.size() / 4);
    while (data_ptr.left())
    {
        *data_ptr++ ^= key;
        rotate_pattern = algo::rotl<u32>(rotate_pattern, 4);
        key = algo::rotr<u32>(key, rotate_pattern);
    }
}

static void decrypt_content_1(bstr &data, u32 key, u32 rotate_pattern)
{
    auto data_ptr = algo::make_ptr(
        data.get<u32>(), std::min<size_t>(0x100, data.size()) / 4);
    while (data_ptr.left())
    {
        *data_ptr++ ^= key;
        rotate_pattern = algo::rotr<u32>(rotate_pattern, 4);
        key = algo::rotl<u32>(key, rotate_pattern);
    }
}

static void decrypt_content_2(bstr &data, const u8 key)
{
    for (auto &c : data)
        c = algo::rotr<u8>(c ^ key, 4);
}

bool LpkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> LpkArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->plugin = plugin_manager.get();

    u32 key1 = meta->plugin.base_key.key1;
    u32 key2 = meta->plugin.base_key.key2;
    {
        const auto base_name = algo::utf8_to_sjis(input_file.path.stem());
        for (const auto i : algo::range(base_name.size()))
        {
            key1 ^= base_name[base_name.size() - 1 - i];
            key2 ^= base_name[i];
            key1 = algo::rotr<u32>(key1, 7);
            key2 = algo::rotl<u32>(key2, 7);
        }
    }

    const auto base_name = algo::lower(input_file.path.stem());
    if (meta->plugin.file_keys.find(base_name) == meta->plugin.file_keys.end())
        throw err::RecognitionError("File not known, can't tell the key");
    const auto aux_key = meta->plugin.file_keys[base_name];
    key1 ^= aux_key.key1;
    key2 ^= aux_key.key2;

    meta->content_key = key1;

    input_file.stream.seek(magic.size());
    const auto tmp = input_file.stream.read_le<u32>() ^ key2;
    meta->flags = static_cast<LpkFlags>(tmp >> 24);
    auto table_size = tmp & 0xFFFFFF;
    if (meta->flags & LpkFlags::AlignedOffset)
        table_size = (table_size << 11) - 8;

    auto table_data = input_file.stream.read(table_size);
    decrypt_table(table_data, key2, meta->plugin.rotate_pattern);

    io::MemoryByteStream input_stream(table_data);
    const auto file_count = input_stream.read_le<u32>();
    const auto prefix_size = input_stream.read<u8>();
    meta->prefix = input_stream.read(prefix_size);
    const auto offset_size = input_stream.read<u8>() ? 4 : 2;
    const auto letter_table_size = input_stream.read_le<u32>();
    const auto entries_offset = input_stream.pos() + letter_table_size;
    const auto entry_size = meta->flags & LpkFlags::IsCompressed ? 13 : 9;

    struct Traversal final
    {
        uoff_t offset;
        std::string current_name;
    };

    struct MiniEntry final
    {
        uoff_t offset;
        std::string name;
    };

    std::vector<MiniEntry> mini_entries;
    std::stack<Traversal> stack;
    stack.push({input_stream.pos(), ""});

    while (!stack.empty())
    {
        const auto traversal = stack.top();
        stack.pop();
        input_stream.seek(traversal.offset);
        const auto entry_count = input_stream.read<u8>();
        for (const auto i : algo::range(entry_count))
        {
            const auto next_letter = input_stream.read<u8>();
            const auto next_offset = offset_size == 4
                ? input_stream.read_le<u32>()
                : input_stream.read_le<u16>();
            if (next_letter == 0)
            {
                mini_entries.push_back({
                    entries_offset + next_offset * entry_size,
                    traversal.current_name});
            }
            else
            {
                stack.push({
                    input_stream.pos() + next_offset,
                    traversal.current_name + std::string(1, next_letter)});
            }
        }
    }

    for (const auto &mini_entry : mini_entries)
    {
        input_stream.seek(mini_entry.offset);
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->flags = input_stream.read<u8>();
        entry->path = mini_entry.name;
        entry->offset = input_stream.read_le<u32>();
        if (meta->flags & LpkFlags::AlignedOffset)
            entry->offset <<= 11;
        entry->size_comp = input_stream.read_le<u32>();
        entry->size_orig = meta->flags & LpkFlags::IsCompressed
            ? input_stream.read_le<u32>()
            : entry->size_comp;
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> LpkArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);

    auto data = input_file.stream
        .seek(entry->offset)
        .read(std::min<size_t>(
            entry->size_comp, input_file.stream.size() - entry->offset));
    data.resize(entry->size_comp);

    if (meta->flags & IsCompressed)
        data = algo::pack::lzss_decompress(data, entry->size_orig);

    if (meta->flags & IsEncrypted2)
        decrypt_content_2(data, meta->plugin.content_xor);

    if (meta->flags & IsEncrypted1)
    {
        decrypt_content_1(
            data, meta->content_key, meta->plugin.rotate_pattern);
    }

    return std::make_unique<io::File>(entry->path, meta->prefix + data);
}

std::vector<std::string> LpkArchiveDecoder::get_linked_formats() const
{
    return {"lucifen/elg"};
}

static auto _ = dec::register_decoder<LpkArchiveDecoder>("lucifen/lpk");
