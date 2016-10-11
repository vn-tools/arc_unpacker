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

#include "dec/french_bread/p_archive_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::french_bread;

bool PArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    Logger dummy_logger;
    dummy_logger.mute();
    auto meta = read_meta(dummy_logger, input_file);
    if (!meta->entries.size())
        return false;
    auto last_entry = static_cast<PlainArchiveEntry*>(
        meta->entries[meta->entries.size() - 1].get());
    return last_entry->offset + last_entry->size == input_file.stream.size();
}

std::unique_ptr<dec::ArchiveMeta> PArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    static const u32 encryption_key = 0xE3DF59AC;
    input_file.stream.seek(0);
    const auto magic = input_file.stream.read_le<u32>();
    const auto file_count = input_file.stream.read_le<u32>() ^ encryption_key;
    if (magic != 0 && magic != 1)
        throw err::RecognitionError();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        auto name = input_file.stream.read(60).str();
        for (const auto j : algo::range(name.size()))
            name[j] ^= i * j * 3 + 0x3D;
        entry->path = name.substr(0, name.find('\0'));
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>() ^ encryption_key;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> PArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto key = entry->path.str();
    const auto encrypted_block_size = std::min<size_t>(0x2173, entry->size);
    auto data = input_file.stream.seek(entry->offset).read(entry->size);
    for (const auto i : algo::range(encrypted_block_size))
        data[i] ^= key[i % key.size()] + i + 3;
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> PArchiveDecoder::get_linked_formats() const
{
    return {"french-bread/ex3"};
}

static auto _ = dec::register_decoder<PArchiveDecoder>("french-bread/p");
