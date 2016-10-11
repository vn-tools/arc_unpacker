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

#include "dec/team_shanghai_alice/pbgz_archive_decoder.h"
#include <map>
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "dec/team_shanghai_alice/crypt.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::team_shanghai_alice;

static const bstr magic = "PBGZ"_b;
static const bstr crypt_magic = "edz"_b;
static const bstr jpeg_magic = "\xFF\xD8\xFF\xE0"_b;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        size_t encryption_version;
    };
}

static std::vector<std::map<u8, DecryptorContext>> decryptors
{
    {
        {0x2A, {0x99, 0x37,  0x400, 0x1000}},
        {0x2D, {0x35, 0x97,   0x80, 0x2800}},
        {0x41, {0xC1, 0x51,  0x400,  0x400}},
        {0x45, {0xAB, 0xCD,  0x200, 0x1000}},
        {0x4A, {0x03, 0x19,  0x400,  0x400}},
        {0x4D, {0x1B, 0x37,   0x40, 0x2800}},
        {0x54, {0x51, 0xE9,   0x40, 0x3000}},
        {0x57, {0x12, 0x34,  0x400,  0x400}},
    },
    {
        {0x2A, {0x99, 0x37,  0x400, 0x1000}},
        {0x2D, {0x35, 0x97,   0x80, 0x2800}},
        {0x41, {0xC1, 0x51, 0x1400, 0x2000}},
        {0x45, {0xAB, 0xCD,  0x200, 0x1000}},
        {0x4A, {0x03, 0x19, 0x1400, 0x7800}},
        {0x4D, {0x1B, 0x37,   0x40, 0x2000}},
        {0x54, {0x51, 0xE9,   0x40, 0x3000}},
        {0x57, {0x12, 0x34,  0x400, 0x2800}},
    },
};

static bstr decompress(const bstr &input, size_t size_orig)
{
    algo::pack::BitwiseLzssSettings settings;
    settings.position_bits = 13;
    settings.size_bits = 4;
    settings.min_match_size = 3;
    settings.initial_dictionary_pos = 1;
    return algo::pack::lzss_decompress(input, size_orig, settings);
}

static std::unique_ptr<io::File> read_file(
    io::File &input_file, const dec::ArchiveEntry &e, u8 encryption_version)
{
    const auto entry = static_cast<const dec::CompressedArchiveEntry*>(&e);

    input_file.stream.seek(entry->offset);
    io::MemoryByteStream uncompressed_stream(
        decompress(
            input_file.stream.read(entry->size_comp),
            entry->size_orig));

    if (uncompressed_stream.read(crypt_magic.size()) != crypt_magic)
        throw err::NotSupportedError("Unknown encryption");

    auto data = decrypt(
        uncompressed_stream.read_to_eof(),
        decryptors[encryption_version][uncompressed_stream.read<u8>()]);

    return std::make_unique<io::File>(entry->path, data);
}

static size_t detect_encryption_version(
    io::File &input_file, const dec::ArchiveMeta &meta)
{
    for (const auto &entry : meta.entries)
    {
        if (!entry->path.has_extension("jpg"))
            continue;
        for (const auto version : algo::range(decryptors.size()))
        {
            auto file = read_file(input_file, *entry, version);
            file->stream.seek(0);
            if (file->stream.read(jpeg_magic.size()) == jpeg_magic)
                return version;
        }
    }
    throw err::NotSupportedError("No means to detect the encryption version");
}

bool PbgzArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> PbgzArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());

    io::MemoryByteStream header_stream(
        decrypt(input_file.stream.read(12), {0x1B, 0x37, 0x0C, 0x400}));
    const auto file_count = header_stream.read_le<u32>() - 123456;
    const auto table_offset = header_stream.read_le<u32>() - 345678;
    const auto table_size_orig = header_stream.read_le<u32>() - 567891;

    input_file.stream.seek(table_offset);
    io::MemoryByteStream table_stream(
        decompress(
            decrypt(input_file.stream.read_to_eof(), {0x3E, 0x9B, 0x80, 0x400}),
            table_size_orig));

    CompressedArchiveEntry *last_entry = nullptr;
    auto meta = std::make_unique<CustomArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CompressedArchiveEntry>();
        entry->path = table_stream.read_to_zero().str();
        entry->offset = table_stream.read_le<u32>();
        entry->size_orig = table_stream.read_le<u32>();
        table_stream.skip(4);
        if (last_entry)
            last_entry->size_comp = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }

    if (last_entry)
        last_entry->size_comp = table_offset - last_entry->offset;

    meta->encryption_version = detect_encryption_version(input_file, *meta);
    return std::move(meta);
}

std::unique_ptr<io::File> PbgzArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    return ::read_file(input_file, e, meta->encryption_version);
}

std::vector<std::string> PbgzArchiveDecoder::get_linked_formats() const
{
    return {"team-shanghai-alice/anm"};
}

static auto _ = dec::register_decoder<PbgzArchiveDecoder>(
    "team-shanghai-alice/pbgz");
