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

#include "dec/kirikiri/xp3_archive_decoder.h"
#include "algo/locale.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::kirikiri;

namespace
{
    struct InfoChunk final
    {
        u32 flags;
        size_t file_size_orig;
        size_t file_size_comp;
        std::string name;
    };

    struct SegmChunk final
    {
        u32 flags;
        uoff_t offset;
        size_t size_orig;
        size_t size_comp;
    };

    struct AdlrChunk final
    {
        u32 key;
    };

    struct TimeChunk final
    {
        u64 timestamp;
    };

    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        Xp3DecryptFunc decrypt_func;
    };

    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        std::unique_ptr<InfoChunk> info_chunk;
        std::vector<std::unique_ptr<SegmChunk>> segm_chunks;
        std::unique_ptr<AdlrChunk> adlr_chunk;
        std::unique_ptr<TimeChunk> time_chunk;
    };
}

static const bstr xp3_magic = "XP3\r\n\x20\x0A\x1A\x8B\x67\x01"_b;
static const bstr hnfn_entry_magic = "hnfn"_b;
static const bstr file_entry_magic = "File"_b;
static const bstr elif_entry_magic = "eliF"_b;
static const bstr info_chunk_magic = "info"_b;
static const bstr segm_chunk_magic = "segm"_b;
static const bstr adlr_chunk_magic = "adlr"_b;
static const bstr time_chunk_magic = "time"_b;

static int detect_version(io::BaseByteStream &input_stream)
{
    if (input_stream.seek(19).read_le<u32>() == 1)
        return 2;
    return 1;
}

static uoff_t get_table_offset(
    io::BaseByteStream &input_stream, const int version)
{
    input_stream.seek(xp3_magic.size());
    if (version == 1)
        return input_stream.read_le<u64>();

    const auto additional_header_offset = input_stream.read_le<u64>();
    const auto minor_version = input_stream.read_le<u32>();
    if (minor_version != 1)
        throw err::CorruptDataError("Unexpected XP3 version");

    input_stream.seek(additional_header_offset);
    input_stream.skip(1); // flags?
    input_stream.skip(8); // table size
    return input_stream.read_le<u64>();
}

static std::unique_ptr<InfoChunk> read_info_chunk(
    io::BaseByteStream &chunk_stream)
{
    auto info_chunk = std::make_unique<InfoChunk>();
    info_chunk->flags = chunk_stream.read_le<u32>();
    info_chunk->file_size_orig = chunk_stream.read_le<u64>();
    info_chunk->file_size_comp = chunk_stream.read_le<u64>();

    const auto file_name_size = chunk_stream.read_le<u16>();
    const auto name = chunk_stream.read(file_name_size * 2);
    info_chunk->name = algo::utf16_to_utf8(name).str();
    return info_chunk;
}

static std::vector<std::unique_ptr<SegmChunk>> read_segm_chunks(
    io::BaseByteStream &chunk_stream)
{
    std::vector<std::unique_ptr<SegmChunk>> segm_chunks;
    while (chunk_stream.left())
    {
        auto segm_chunk = std::make_unique<SegmChunk>();
        segm_chunk->flags = chunk_stream.read_le<u32>();
        segm_chunk->offset = chunk_stream.read_le<u64>();
        segm_chunk->size_orig = chunk_stream.read_le<u64>();
        segm_chunk->size_comp = chunk_stream.read_le<u64>();
        segm_chunks.push_back(std::move(segm_chunk));
    }
    return segm_chunks;
}

static std::unique_ptr<AdlrChunk> read_adlr_chunk(
    io::BaseByteStream &chunk_stream)
{
    auto adlr_chunk = std::make_unique<AdlrChunk>();
    adlr_chunk->key = chunk_stream.read_le<u32>();
    return adlr_chunk;
}

static std::unique_ptr<TimeChunk> read_time_chunk(
    io::BaseByteStream &chunk_stream)
{
    auto time_chunk = std::make_unique<TimeChunk>();
    time_chunk->timestamp = chunk_stream.read_le<u64>();
    return time_chunk;
}

static void read_hnfn_entry(
    io::BaseByteStream &input_stream,
    std::map<u32, std::string> &fn_map)
{
    const auto hash = input_stream.read_le<u32>();
    const auto name_size = input_stream.read_le<u16>();
    fn_map[hash] = algo::utf16_to_utf8(input_stream.read(name_size * 2)).str();
}

static void read_elif_entry(
    io::BaseByteStream &input_stream,
    std::map<u32, std::string> &fn_map)
{
    const auto hash = input_stream.read_le<u32>();
    const auto name_size = input_stream.read_le<u16>();
    fn_map[hash] = algo::utf16_to_utf8(input_stream.read(name_size * 2)).str();
}

static std::unique_ptr<CustomArchiveEntry> read_file_entry(
    const Logger &logger,
    io::BaseByteStream &input_stream,
    const std::map<u32, std::string> &fn_map)
{
    auto entry = std::make_unique<CustomArchiveEntry>();
    while (input_stream.left())
    {
        const auto chunk_magic = input_stream.read(4);
        const auto chunk_size = input_stream.read_le<u64>();
        io::MemoryByteStream chunk_stream(input_stream.read(chunk_size));

        if (chunk_magic == info_chunk_magic)
            entry->info_chunk = read_info_chunk(chunk_stream);
        else if (chunk_magic == segm_chunk_magic)
            entry->segm_chunks = read_segm_chunks(chunk_stream);
        else if (chunk_magic == adlr_chunk_magic)
            entry->adlr_chunk = read_adlr_chunk(chunk_stream);
        else if (chunk_magic == time_chunk_magic)
            entry->time_chunk = read_time_chunk(chunk_stream);
        else
        {
            logger.warn("Unknown chunk '%s'\n", chunk_magic.c_str());
            continue;
        }

        if (chunk_stream.left())
        {
            logger.warn(
                "'%s' chunk contains data beyond EOF\n", chunk_magic.c_str());
        }
    }
    if (input_stream.left())
        throw err::CorruptDataError("FILE entry contains data beyond EOF");

    if (!entry->info_chunk)
        throw err::CorruptDataError("INFO chunk not found");
    if (!entry->adlr_chunk)
        throw err::CorruptDataError("ADLR chunk not found");
    if (entry->segm_chunks.empty())
        throw err::CorruptDataError("No SEGM chunks found");

    entry->path = fn_map.find(entry->adlr_chunk->key) != fn_map.end()
        ? fn_map.at(entry->adlr_chunk->key)
        : entry->info_chunk->name;

    return entry;
}

bool Xp3ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(xp3_magic.size()) == xp3_magic;
}

std::unique_ptr<dec::ArchiveMeta> Xp3ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto version = detect_version(input_file.stream);
    const auto table_offset = get_table_offset(input_file.stream, version);

    input_file.stream.seek(table_offset);
    const auto table_is_compressed = input_file.stream.read<u8>() != 0;
    const auto table_size_comp = input_file.stream.read_le<u64>();
    const auto table_size_orig = table_is_compressed
        ? input_file.stream.read_le<u64>()
        : table_size_comp;

    auto table_data = input_file.stream.read(table_size_comp);
    if (table_is_compressed)
        table_data = algo::pack::zlib_inflate(table_data);
    io::MemoryByteStream table_stream(table_data);

    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->decrypt_func = plugin_manager.get()
        .create_decrypt_func(input_file.path);

    std::map<u32, std::string> fn_map;
    while (table_stream.left())
    {
        const auto entry_magic = table_stream.read(4);
        const auto entry_size = table_stream.read_le<u64>();
        io::MemoryByteStream entry_stream(table_stream.read(entry_size));

        if (entry_magic == file_entry_magic)
            meta->entries.push_back(
                read_file_entry(logger, entry_stream, fn_map));
        else if (entry_magic == hnfn_entry_magic)
            read_hnfn_entry(entry_stream, fn_map);
        else if (entry_magic == elif_entry_magic)
            read_elif_entry(entry_stream, fn_map);
        else
            throw err::NotSupportedError("Unknown entry: " + entry_magic.str());
    }
    return std::move(meta);
}

std::unique_ptr<io::File> Xp3ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);

    bstr data;
    for (const auto &segm_chunk : entry->segm_chunks)
    {
        const auto data_is_compressed = segm_chunk->flags & 7;
        input_file.stream.seek(segm_chunk->offset);
        data += data_is_compressed
            ? algo::pack::zlib_inflate(
                input_file.stream.read(segm_chunk->size_comp))
            : input_file.stream.read(segm_chunk->size_orig);
    }

    if (meta->decrypt_func)
        meta->decrypt_func(data, entry->adlr_chunk->key);

    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Xp3ArchiveDecoder::get_linked_formats() const
{
    return {"kirikiri/tlg"};
}

static auto _ = dec::register_decoder<Xp3ArchiveDecoder>("kirikiri/xp3");
