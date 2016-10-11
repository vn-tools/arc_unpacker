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

#include "dec/team_shanghai_alice/tha1_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "dec/team_shanghai_alice/crypt.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::team_shanghai_alice;

static const bstr magic = "THA1"_b;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        size_t encryption_version;
    };

    struct CustomArchiveEntry  final : dec::CompressedArchiveEntry
    {
        u8 decryptor_id;
    };
}

static std::vector<std::vector<DecryptorContext>> decryptors
{
    // TH9.5, TH10, TH11
    {
        {0x1B, 0x37,  0x40, 0x2800},
        {0x51, 0xE9,  0x40, 0x3000},
        {0xC1, 0x51,  0x80, 0x3200},
        {0x03, 0x19, 0x400, 0x7800},
        {0xAB, 0xCD, 0x200, 0x2800},
        {0x12, 0x34,  0x80, 0x3200},
        {0x35, 0x97,  0x80, 0x2800},
        {0x99, 0x37, 0x400, 0x2000},
    },

    // TH12, TH12.5, TH12.8
    {
        {0x1B, 0x73,  0x40, 0x3800},
        {0x51, 0x9E,  0x40, 0x4000},
        {0xC1, 0x15, 0x400, 0x2C00},
        {0x03, 0x91,  0x80, 0x6400},
        {0xAB, 0xDC,  0x80, 0x6E00},
        {0x12, 0x43, 0x200, 0x3C00},
        {0x35, 0x79, 0x400, 0x3C00},
        {0x99, 0x7D,  0x80, 0x2800},
    },

    // TH13
    {
        {0x1B, 0x73, 0x0100, 0x3800},
        {0x12, 0x43, 0x0200, 0x3E00},
        {0x35, 0x79, 0x0400, 0x3C00},
        {0x03, 0x91, 0x0080, 0x6400},
        {0xAB, 0xDC, 0x0080, 0x6E00},
        {0x51, 0x9E, 0x0100, 0x4000},
        {0xC1, 0x15, 0x0400, 0x2C00},
        {0x99, 0x7D, 0x0080, 0x4400},
    },

    // TH14, TH15 trial, TH15
    {
        {0x1B, 0x73, 0x0100, 0x3800},
        {0x12, 0x43, 0x0200, 0x3E00},
        {0x35, 0x79, 0x0400, 0x3C00},
        {0x03, 0x91, 0x0080, 0x6400},
        {0xAB, 0xDC, 0x0080, 0x7000},
        {0x51, 0x9E, 0x0100, 0x4000},
        {0xC1, 0x15, 0x0400, 0x2C00},
        {0x99, 0x7D, 0x0080, 0x4400}
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

static int detect_encryption_version(io::File &input_file)
{
    const auto name = input_file.path.stem();
    if (name == "th095" || name == "th095e") return 0;
    if (name == "th10"  || name == "th10e")  return 0;
    if (name == "th11"  || name == "th11e")  return 0;
    if (name == "th12"  || name == "th12e")  return 1;
    if (name == "th125" || name == "th125e") return 1;
    if (name == "th128" || name == "th128e") return 1;
    if (name == "th13"  || name == "th13e")  return 2;
    if (name == "th14"  || name == "th14e")  return 3;
    if (name == "th143" || name == "th143e") return 3;
    if (name == "th15"  || name == "th15e" || name == "th15tr") return 3;
    return -1;
}

bool Tha1ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("dat"))
        return false;
    const auto encryption_version = detect_encryption_version(input_file);
    if (encryption_version < 0)
        return false;
    input_file.stream.seek(0);
    auto header_data = input_file.stream.read(16);
    header_data = decrypt(header_data, {0x1B, 0x37, 0x10, 0x400});
    io::MemoryByteStream header_stream(header_data);
    return header_stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Tha1ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto header_data = input_file.stream.read(16);
    header_data = decrypt(header_data, {0x1B, 0x37, 0x10, 0x400});
    io::MemoryByteStream header_stream(header_data);
    if (header_stream.read(magic.size()) != magic)
        throw err::RecognitionError();
    const auto table_size_orig = header_stream.read_le<u32>() - 123456789;
    const auto table_size_comp = header_stream.read_le<u32>() - 987654321;
    const auto file_count = header_stream.read_le<u32>() - 135792468;
    const auto table_offset = input_file.stream.size() - table_size_comp;

    input_file.stream.seek(table_offset);
    auto table_data = input_file.stream.read(table_size_comp);
    table_data = decrypt(table_data, {0x3E, 0x9B, 0x80, table_size_comp});
    table_data = decompress(table_data, table_size_orig);
    io::MemoryByteStream table_stream(table_data);

    CustomArchiveEntry *last_entry = nullptr;
    auto meta = std::make_unique<CustomArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();

        entry->path = table_stream.read_to_zero().str();
        table_stream.skip(3 - entry->path.str().size() % 4);

        entry->decryptor_id = 0;
        for (const auto &c : entry->path.str())
            entry->decryptor_id += c;
        entry->decryptor_id %= 8;

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

    meta->encryption_version = detect_encryption_version(input_file);
    return std::move(meta);
}

std::unique_ptr<io::File> Tha1ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);

    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);
    data = decrypt(
        data, decryptors[meta->encryption_version][entry->decryptor_id]);
    if (entry->size_comp != entry->size_orig)
        data = decompress(data, entry->size_orig);

    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Tha1ArchiveDecoder::get_linked_formats() const
{
    return {"team-shanghai-alice/anm"};
}

static auto _ = dec::register_decoder<Tha1ArchiveDecoder>(
    "team-shanghai-alice/tha1");
