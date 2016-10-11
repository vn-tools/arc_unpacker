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

#include "dec/leaf/kcap_group/kcap_archive_decoder.h"
#include "algo/format.h"
#include "algo/locale.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "KCAP"_b;

namespace
{
    enum class EntryType : u32
    {
        RegularFile    = 0x00000000,
        CompressedFile = 0x00000001,
    };

    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        bool compressed;
    };
}

static size_t detect_version(io::File &input_file, const size_t file_count)
{
    size_t version = 0;
    input_file.stream.peek(input_file.stream.pos(), [&]()
    {
        input_file.stream.skip((file_count - 1) * (24 + 8));
        input_file.stream.skip(24);
        const auto last_entry_offset = input_file.stream.read_le<u32>();
        const auto last_entry_size = input_file.stream.read_le<u32>();
        if (last_entry_offset + last_entry_size == input_file.stream.size())
            version = 1;
    });
    input_file.stream.peek(input_file.stream.pos(), [&]()
    {
        input_file.stream.skip((file_count - 1) * (4 + 24 + 8));
        input_file.stream.skip(4 + 24);
        const auto last_entry_offset = input_file.stream.read_le<u32>();
        const auto last_entry_size = input_file.stream.read_le<u32>();
        if (last_entry_offset + last_entry_size == input_file.stream.size())
            version = 2;
    });
    return version;
}

static std::unique_ptr<dec::ArchiveMeta> read_meta_v1(
    io::File &input_file, const size_t file_count)
{
    auto meta = std::make_unique<dec::ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->compressed = true;
        entry->path = algo::sjis_to_utf8(
            input_file.stream.read_to_zero(24)).str();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

static std::unique_ptr<dec::ArchiveMeta> read_meta_v2(
    io::File &input_file, const size_t file_count, const Logger &logger)
{
    auto meta = std::make_unique<dec::ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        const auto type = static_cast<EntryType>(
            input_file.stream.read_le<u32>());
        entry->path = algo::sjis_to_utf8(
            input_file.stream.read_to_zero(24)).str();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        if (type == EntryType::RegularFile)
            entry->compressed = false;
        else if (type == EntryType::CompressedFile)
            entry->compressed = true;
        else
        {
            if (!entry->size)
                continue;
            logger.warn("Unknown entry type: %08x\n", type);
        }
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

bool KcapArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> KcapArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    const auto version = detect_version(input_file, file_count);
    if (version == 1)
        return read_meta_v1(input_file, file_count);
    else if (version == 2)
        return read_meta_v2(input_file, file_count, logger);
    else
        throw err::UnsupportedVersionError(version);
}

std::unique_ptr<io::File> KcapArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);
    bstr data;
    if (entry->compressed)
    {
        const auto size_comp = input_file.stream.read_le<u32>();
        const auto size_orig = input_file.stream.read_le<u32>();
        data = input_file.stream.read(size_comp - 8);
        data = algo::pack::lzss_decompress(data, size_orig);
    }
    else
        data = input_file.stream.read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> KcapArchiveDecoder::get_linked_formats() const
{
    return {"truevision/tga", "leaf/bbm", "leaf/bjr"};
}

static auto _ = dec::register_decoder<KcapArchiveDecoder>("leaf/kcap");
