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

#include "dec/dxlib/dx_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::dxlib;

static const auto magic = "DX"_b;
static const auto known_keys =
{
    "\x55\xAA\x20\x55\x55\x06\x55\xAA\x55\xD5\x7C\x66"_b,
    "\x90\xD6\xEB\x19\x9C\xC3\x90\x52\x0B\x11\xE0\xA3"_b,
    "\x99\x96\xF8\xA9\x88\xC3\x8D\x92\x33\x16\xF1\xA9"_b,
    "\xA0\x47\xEB\xC8\x94\xCA\x90\xB1\x1B\x1A\x23\x93"_b,
    "\x97\x57\xED\x0A\xCF\x9C\xCE\xF2\xAB\x18\x23\xFC"_b,
    "\xB9\x26\xEB\x98\xAF\xE7\xBC\x95\xB9\x39\xF0\xAD"_b,
    "\x95\x96\xF8\x09\x8B\xDF\x8A\x92\x2B\x15\x40\xBE"_b,
};

namespace
{
    struct Header final
    {
        u16 version;
        size_t table_size;
        uoff_t content_offset;
        uoff_t table_offset;
        uoff_t file_table_offset;
        uoff_t dir_table_offset;
    };

    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        bstr key;
    };
}

static bstr decrypt(
    io::BaseByteStream &input_stream, size_t size, const bstr &key)
{
    auto j = input_stream.pos();
    auto ret = input_stream.read(size);
    for (const auto i : algo::range(size))
    {
        ret[i] ^= key[j % key.size()];
        j++;
    }
    return ret;
}

static bstr detect_key(io::BaseByteStream &input_stream)
{
    if (input_stream.size() > 4)
    {
        const auto maybe_magic = input_stream.seek(0).read(2);
        for (const auto &key : known_keys)
        {
            auto unencrypted_magic = maybe_magic.substr(0, 2);
            unencrypted_magic[0] ^= key[0];
            unencrypted_magic[1] ^= key[1];
            if (unencrypted_magic == magic)
                return key;
        }
    }
    return ""_b;
}

static Header read_header(io::BaseByteStream &input_stream, const bstr &key)
{
    input_stream.seek(0);
    io::MemoryByteStream header_stream(decrypt(input_stream, 24, key));
    header_stream.skip(magic.size());
    Header header;
    header.version = header_stream.read_le<u16>();
    header.table_size = header_stream.read_le<u32>();
    header.content_offset = header_stream.read_le<u32>();
    header.table_offset = header_stream.read_le<u32>();
    header.file_table_offset = header_stream.read_le<u32>();
    header.dir_table_offset = header_stream.read_le<u32>();
    return header;
}

static std::string read_file_name(
    io::BaseByteStream &input_stream, const uoff_t offset)
{
    std::string ret;
    input_stream.peek(input_stream.pos(), [&]()
    {
        input_stream.seek(offset);
        const auto name_offset = input_stream.read_le<u16>() * 4 + 4;
        input_stream.seek(offset + name_offset);
        ret = algo::sjis_to_utf8(input_stream.read_to_zero()).str();
    });
    return ret;
}

static void read_file_table(
    io::BaseByteStream &table_stream,
    const Header &header,
    const uoff_t initial_offset,
    io::path root_path,
    CustomArchiveMeta &meta)
{
    table_stream.seek(header.dir_table_offset + initial_offset);
    const auto dir_offset = table_stream.read_le<s32>();
    const auto parent_dir_offset = table_stream.read_le<s32>();
    const auto file_count = table_stream.read_le<u32>();
    const auto file_table_offset = table_stream.read_le<u32>();
    if (dir_offset != -1 && parent_dir_offset != -1)
    {
        table_stream.seek(header.file_table_offset + dir_offset);
        const auto name_offset = table_stream.read_le<u32>();
        root_path /= read_file_name(table_stream, name_offset);
    }

    table_stream.seek(header.file_table_offset + file_table_offset);
    for (const auto i : algo::range(file_count))
    {
        const auto name_offset = table_stream.read_le<u32>();
        const auto flags = table_stream.read_le<u32>();
        table_stream.skip(0x18);
        const auto entry_offset = table_stream.read_le<u32>();
        if (flags & 0x10)
        {
            read_file_table(
                table_stream,
                header,
                entry_offset,
                root_path,
                meta);
        }
        else
        {
            auto entry = std::make_unique<dec::CompressedArchiveEntry>();
            entry->path
                = root_path / read_file_name(table_stream, name_offset);
            entry->offset = header.content_offset + entry_offset;
            entry->size_orig = table_stream.read_le<u32>();
            entry->size_comp = header.version >= 2
                ? table_stream.read_le<u32>()
                : -1;
            meta.entries.push_back(std::move(entry));
        }
    }
}

bool DxArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return !detect_key(input_file.stream).empty();
}

std::unique_ptr<dec::ArchiveMeta> DxArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto key = detect_key(input_file.stream);
    const auto header = read_header(input_file.stream, key);

    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->key = key;

    input_file.stream.seek(header.table_offset);
    io::MemoryByteStream table_stream(
        decrypt(input_file.stream, header.table_size, key));
    read_file_table(table_stream, header, 0, "", *meta);

    return std::move(meta);
}

std::unique_ptr<io::File> DxArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CompressedArchiveEntry*>(&e);
    const auto is_compressed = static_cast<s32>(entry->size_comp) != -1;
    const auto data = decrypt(
        input_file.stream.seek(entry->offset),
        is_compressed ? entry->size_comp : entry->size_orig,
        meta->key);
    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<DxArchiveDecoder>("dxlib/dx");
