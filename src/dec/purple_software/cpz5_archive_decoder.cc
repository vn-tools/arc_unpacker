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

#include "dec/purple_software/cpz5_archive_decoder.h"
#include "algo/binary.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "dec/purple_software/cpz5/crypt.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::purple_software;

static const bstr magic5 = "CPZ5"_b;
static const bstr magic6 = "CPZ6"_b;

namespace
{
    struct Header final
    {
        size_t dir_count;
        size_t dir_table_size;
        size_t file_table_size;
        std::array<u32, 4> md5_dwords;
        u32 main_key;
        u32 extra_key;
        size_t table_size;
        size_t data_start;
    };

    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        cpz5::Plugin plugin;
        u32 main_key;
        std::array<u32, 4> hash;
    };

    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        u32 key;
    };

    struct DirectoryInfo final
    {
        size_t file_count;
        uoff_t file_table_offset;
        size_t file_table_size;
        u32 file_table_main_key;
        io::path path;
    };
}

static std::unique_ptr<CustomArchiveMeta> read_meta(
    const cpz5::Plugin &plugin, const Header &header, const bstr &table_data)
{
    if (!header.dir_table_size || !header.file_table_size)
        throw err::BadDataSizeError();
    const auto hash = cpz5::get_hash(plugin, header.md5_dwords);
    bstr table_data_copy(table_data);
    auto table_data_ptr = algo::make_ptr(table_data_copy);
    auto dir_table_ptr = algo::make_ptr(
        &table_data_copy[0], header.dir_table_size);
    auto file_table_ptr = algo::make_ptr(
        &table_data_copy[header.dir_table_size], header.file_table_size);

    // whole index
    cpz5::decrypt_1a(table_data_ptr, plugin, header.main_key ^ 0x3795B39A);

    // just directories
    cpz5::decrypt_2(dir_table_ptr, plugin, header.main_key, hash[1], 0x3A);
    cpz5::decrypt_1b(dir_table_ptr, header.main_key, hash);

    DirectoryInfo *prev_dir = nullptr;
    std::vector<std::unique_ptr<DirectoryInfo>> dirs;
    io::MemoryByteStream dir_table_stream(table_data_copy);
    for (const auto i : algo::range(header.dir_count))
    {
        auto dir = std::make_unique<DirectoryInfo>();
        const auto entry_offset = dir_table_stream.pos();
        const auto entry_size = dir_table_stream.read_le<u32>();
        dir->file_count = dir_table_stream.read_le<u32>();
        dir->file_table_offset = dir_table_stream.read_le<u32>();
        dir->file_table_main_key = dir_table_stream.read_le<u32>();
        dir->path = algo::sjis_to_utf8(dir_table_stream.read_to_zero()).str();
        if (prev_dir)
        {
            prev_dir->file_table_size
                = dir->file_table_offset - prev_dir->file_table_offset;
        }
        prev_dir = dir.get();
        dirs.push_back(std::move(dir));
        dir_table_stream.seek(entry_offset + entry_size);
    }
    if (prev_dir)
    {
        prev_dir->file_table_size = header.table_size
            - header.dir_table_size
            - prev_dir->file_table_offset;
    }

    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->plugin = plugin;
    meta->main_key = header.main_key;
    meta->hash = hash;

    for (const auto &dir : dirs)
    {
        auto dir_file_table_ptr = algo::make_ptr(
            file_table_ptr + dir->file_table_offset, dir->file_table_size);

        cpz5::decrypt_2(
            dir_file_table_ptr, plugin, header.main_key, hash[2], 0x7E);
        cpz5::decrypt_1c(
            dir_file_table_ptr, plugin, dir->file_table_main_key, hash);

        io::MemoryByteStream file_table_stream(
            bstr(&dir_file_table_ptr[0], dir_file_table_ptr.size()));

        for (const auto i : algo::range(dir->file_count))
        {
            auto entry = std::make_unique<CustomArchiveEntry>();
            const auto entry_offset = file_table_stream.pos();
            const auto entry_size = file_table_stream.read_le<u32>();
            const auto relative_offset = file_table_stream.read_le<u64>();
            entry->offset = relative_offset + header.data_start;
            entry->size = file_table_stream.read_le<u32>();
            file_table_stream.skip(4);
            const auto tmp = file_table_stream.read_le<u32>();
            entry->key
                = ((header.main_key ^ (dir->file_table_main_key + tmp))
                + header.dir_count - 0x5C29E87B) ^ header.extra_key;
            entry->path
                = algo::sjis_to_utf8(file_table_stream.read_to_zero()).str();
            if (dir->path.name() != "root")
                entry->path = dir->path / entry->path;
            meta->entries.push_back(std::move(entry));
            file_table_stream.seek(entry_offset + entry_size);
        }
    }
    return meta;
}

static Header read_header_5(io::BaseByteStream &input_stream)
{
    Header header;
    header.dir_count = input_stream.read_le<u32>() ^ 0xFE3A53D9;
    header.dir_table_size = input_stream.read_le<u32>() ^ 0x37F298E7;
    header.file_table_size = input_stream.read_le<u32>() ^ 0x7A6F3A2C;
    input_stream.skip(16);
    header.md5_dwords[0] = input_stream.read_le<u32>() ^ 0x43DE7C19;
    header.md5_dwords[1] = input_stream.read_le<u32>() ^ 0xCC65F415;
    header.md5_dwords[2] = input_stream.read_le<u32>() ^ 0xD016A93C;
    header.md5_dwords[3] = input_stream.read_le<u32>() ^ 0x97A3BA9A;
    header.main_key = input_stream.read_le<u32>() ^ 0xAE7D39BF;
    header.extra_key = 0;
    input_stream.skip(12);
    header.table_size = header.dir_table_size + header.file_table_size;
    header.data_start = input_stream.pos() + header.table_size;
    return header;
}

static Header read_header_6(io::BaseByteStream &input_stream)
{
    Header header;
    header.dir_count = input_stream.read_le<u32>() ^ 0xFE3A53DA;
    header.dir_table_size = input_stream.read_le<u32>() ^ 0x37F298E8;
    header.file_table_size = input_stream.read_le<u32>() ^ 0x7A6F3A2D;
    input_stream.skip(16);
    header.md5_dwords[0] = input_stream.read_le<u32>() ^ 0x43DE7C1A;
    header.md5_dwords[1] = input_stream.read_le<u32>() ^ 0xCC65F416;
    header.md5_dwords[2] = input_stream.read_le<u32>() ^ 0xD016A93D;
    header.md5_dwords[3] = input_stream.read_le<u32>() ^ 0x97A3BA9B;
    header.main_key = input_stream.read_le<u32>() ^ 0xAE7D39B7;
    const auto tmp1 = input_stream.read_le<u32>() ^ 0xFB73A956;
    const auto tmp2 = input_stream.read_le<u32>() ^ 0x37ACF832;
    header.extra_key = (algo::rotr(tmp2, 5) * 0x7DA8F173) + 0x13712765;
    input_stream.skip(4);
    header.table_size = header.dir_table_size + header.file_table_size;
    header.data_start = input_stream.pos() + header.table_size;
    return header;
}

struct Cpz5ArchiveDecoder::Priv final
{
    size_t version;
};

Cpz5ArchiveDecoder::Cpz5ArchiveDecoder(const size_t version)
    : p(new Priv {version})
{
}

Cpz5ArchiveDecoder::~Cpz5ArchiveDecoder()
{
}

bool Cpz5ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (p->version == 5)
        return input_file.stream.seek(0).read(magic5.size()) == magic5;
    else if (p->version == 6)
        return input_file.stream.seek(0).read(magic6.size()) == magic6;
    else
        throw std::logic_error("Bad version");
}

std::unique_ptr<dec::ArchiveMeta> Cpz5ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    std::vector<std::shared_ptr<cpz5::Plugin>> plugins;
    Header header;

    if (p->version == 5)
    {
        input_file.stream.seek(magic5.size());
        header = read_header_5(input_file.stream);
        plugins = cpz5::get_cpz5_plugins();
    }
    else if (p->version == 6)
    {
        input_file.stream.seek(magic6.size());
        header = read_header_6(input_file.stream);
        plugins = cpz5::get_cpz6_plugins();
    }
    else
    {
        throw std::logic_error("Bad version");
    }

    auto table_data = input_file.stream.read(header.table_size);
    for (const auto &plugin : plugins)
    {
        try
        {
            auto meta = ::read_meta(*plugin, header, table_data);
            if (meta)
                return std::move(meta);
        }
        catch (const err::DataError)
        {
            continue;
        }
        catch (const err::IoError)
        {
            continue;
        }
    }
    throw err::NotSupportedError("Unsupported encryption");
}

std::unique_ptr<io::File> Cpz5ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size);

    cpz5::decrypt_3(
        algo::make_ptr(data),
        meta->plugin,
        meta->hash[3],
        meta->main_key,
        meta->hash,
        entry->key);

    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Cpz5ArchiveDecoder::get_linked_formats() const
{
    return {"purple-software/ps2", "purple-software/pb3"};
}

static auto dummy1
    = dec::register_decoder<Cpz5ArchiveDecoder>("purple-software/cpz5", 5);

static auto dummy2
    = dec::register_decoder<Cpz5ArchiveDecoder>("purple-software/cpz6", 6);
