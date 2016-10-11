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

#include "dec/shiina_rio/warc_archive_decoder.h"
#include <set>
#include "algo/str.h"
#include "dec/shiina_rio/warc/decompress.h"
#include "dec/shiina_rio/warc/decrypt.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::shiina_rio;

static const bstr magic = "WARC\x20"_b;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        CustomArchiveMeta(
            const std::shared_ptr<warc::Plugin> plugin,
            const int warc_version);

        const std::shared_ptr<warc::Plugin> plugin;
        const int warc_version;
    };

    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        u64 time;
        u32 flags;
        bool suspicious;
    };
}

CustomArchiveMeta::CustomArchiveMeta(
    const std::shared_ptr<warc::Plugin> plugin, const int warc_version)
    : plugin(plugin), warc_version(warc_version)
{
}

bool WarcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> WarcArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto plugin = plugin_manager.get()();
    input_file.stream.seek(magic.size());
    const int warc_version = 100
        * algo::from_string<float>(input_file.stream.read(3).str());
    if (warc_version != 170)
        throw err::UnsupportedVersionError(warc_version);

    input_file.stream.seek(8);
    const auto table_offset = input_file.stream.read_le<u32>() ^ 0xF182AD82;
    input_file.stream.seek(table_offset);
    auto table_data = input_file.stream.read_to_eof();

    warc::decrypt_table_data(*plugin, warc_version, table_offset, table_data);
    io::MemoryByteStream table_stream(table_data);

    std::set<io::path> known_names;
    auto meta = std::make_unique<CustomArchiveMeta>(plugin, warc_version);
    while (table_stream.left())
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        auto name = table_stream.read_to_zero(plugin->entry_name_size).str();
        for (auto &c : name)
        {
            if (static_cast<const u8>(c) >= 0x80)
            {
                c = '_';
                entry->suspicious = true;
            }
        }
        entry->path = name;
        entry->suspicious |= known_names.find(entry->path) != known_names.end();
        known_names.insert(entry->path);
        entry->offset = table_stream.read_le<u32>();
        entry->size_comp = table_stream.read_le<u32>();
        entry->size_orig = table_stream.read_le<u32>();
        entry->time = table_stream.read_le<u64>();
        entry->flags = table_stream.read_le<u32>();
        if (!entry->path.str().empty())
            meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> WarcArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);

    const auto head = input_file.stream.read(4);
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto compress_crypt = head[3] > 0;
    const u32 tmp = *head.get<u32>() ^ 0x82AD82 ^ size_orig;
    bstr file_magic(3);
    file_magic[0] = tmp & 0xFF;
    file_magic[1] = (tmp >> 8) & 0xFF;
    file_magic[2] = (tmp >> 16) & 0xFF;

    auto data = input_file.stream.read(entry->size_comp - 8);
    if (entry->flags & 0x80000000)
        warc::decrypt_essential(*meta->plugin, meta->warc_version, data);
    if (meta->plugin->extra_crypt)
        meta->plugin->extra_crypt->decrypt(data, 0x202);
    if (entry->flags & 0x20000000)
    {
        if (!meta->plugin->crc_crypt_source.size())
            throw err::NotSupportedError("CRC crypt is not implemented");
        warc::crc_crypt(data, meta->plugin->crc_crypt_source);
    }

    std::function<bstr(const bstr &, const size_t, const bool)> decompressor;
    if (file_magic == "YH1"_b)
        decompressor = warc::decompress_yh1;
    else if (file_magic == "YLZ"_b)
        decompressor = warc::decompress_ylz;
    else if (file_magic == "YPK"_b)
        decompressor = warc::decompress_ypk;

    if (decompressor)
    {
        data = decompressor(data, size_orig, compress_crypt);
        if (entry->flags & 0x40000000)
        {
            if (!meta->plugin->crc_crypt_source.size())
                throw err::NotSupportedError("CRC crypt is not implemented");
            warc::crc_crypt(data, meta->plugin->crc_crypt_source);
        }
        if (meta->plugin->extra_crypt)
            meta->plugin->extra_crypt->decrypt(data, 0x204);
    }

    if (entry->suspicious)
    {
        logger.warn(
            "Suspicious entry: %s (anti-extract decoy?)\n",
            entry->path.c_str());
    }

    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> WarcArchiveDecoder::get_linked_formats() const
{
    return {"shiina-rio/ogv", "shiina-rio/s25"};
}

static auto _ = dec::register_decoder<WarcArchiveDecoder>("shiina-rio/warc");
