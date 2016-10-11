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

#include "dec/nitroplus/npk2_archive_decoder.h"
#include "algo/crypt/aes.h"
#include "algo/locale.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::nitroplus;

static const auto magic = "NPK2"_b;

namespace
{
    struct Segment final
    {
        uoff_t offset;
        size_t size_aligned;
        size_t size_comp;
        size_t size_orig;
    };

    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        bstr iv;
        bstr key;
    };

    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        std::vector<Segment> segments;
    };
}

static bstr decrypt(const bstr &input, const CustomArchiveMeta &meta)
{
    return algo::crypt::aes256_decrypt_cbc(input, meta.iv, meta.key);
}

bool Npk2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Npk2ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(8);

    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->iv = input_file.stream.read(16);
    meta->key = plugin_manager.get();

    const auto file_count = input_file.stream.read_le<u32>();
    const auto table_size = input_file.stream.read_le<u32>();

    auto table = decrypt(input_file.stream.read(table_size), *meta);
    io::MemoryByteStream table_stream(table);

    for (const auto i : algo::range(file_count))
    {
        table_stream.skip(1);
        const auto name_size = table_stream.read_le<u16>();
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = algo::sjis_to_utf8(table_stream.read(name_size)).str();
        table_stream.skip(36);
        const auto segment_count = table_stream.read_le<u32>();
        for (const auto j : algo::range(segment_count))
        {
            Segment segment;
            segment.offset = table_stream.read_le<u64>();
            segment.size_aligned = table_stream.read_le<u32>();
            segment.size_comp = table_stream.read_le<u32>();
            segment.size_orig = table_stream.read_le<u32>();
            entry->segments.push_back(segment);
        }
        meta->entries.push_back(std::move(entry));
    }

    return std::move(meta);
}

std::unique_ptr<io::File> Npk2ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    auto output_file = std::make_unique<io::File>(entry->path, ""_b);

    for (const auto &segment : entry->segments)
    {
        auto segment_data = input_file.stream
            .seek(segment.offset)
            .read(segment.size_aligned);
        segment_data = decrypt(segment_data, *meta);
        if (segment.size_orig > segment.size_comp)
        {
            segment_data = algo::pack::zlib_inflate(
                segment_data, algo::pack::ZlibKind::RawDeflate);
        }
        output_file->stream.write(segment_data);
    }
    return output_file;
}

std::vector<std::string> Npk2ArchiveDecoder::get_linked_formats() const
{
    return {"microsoft/dds"};
}

static auto _ = dec::register_decoder<Npk2ArchiveDecoder>("nitroplus/npk2");
