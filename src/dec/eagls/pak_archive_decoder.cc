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

#include "dec/eagls/pak_archive_decoder.h"
#include "algo/crypt/lcg.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "io/file_byte_stream.h"
#include "io/file_system.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::eagls;

static const bstr key = "1qaz2wsx3edc4rfv5tgb6yhn7ujm8ik,9ol.0p;/-@:^[]"_b;

static io::path get_path_to_index(const io::path &path_to_data)
{
    io::path index_path(path_to_data);
    index_path.change_extension("idx");
    return index_path;
}

bool PakArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return io::exists(get_path_to_index(input_file.path))
        && input_file.path.has_extension("pak");
}

std::unique_ptr<dec::ArchiveMeta> PakArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    io::FileByteStream index_stream(
        get_path_to_index(input_file.path), io::FileMode::Read);

    auto data = index_stream.read(index_stream.size() - 4);
    const auto seed = index_stream.read_le<u32>();
    algo::crypt::Lcg lcg(algo::crypt::LcgKind::MicrosoftVisualC, seed);
    for (const auto i : algo::range(data.size()))
        data[i] ^= key[lcg.next() % key.size()];

    io::MemoryByteStream data_stream(data);
    uoff_t min_offset = std::numeric_limits<uoff_t>::max();
    auto meta = std::make_unique<ArchiveMeta>();
    while (true)
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = algo::sjis_to_utf8(data_stream.read_to_zero(20)).str();
        if (entry->path.str().empty())
            break;
        entry->offset = data_stream.read_le<u32>();
        entry->size = data_stream.read_le<u32>();
        min_offset = std::min(min_offset, entry->offset);
        meta->entries.push_back(std::move(entry));
    }

    // According to Crass min_offset is calculated differently for some games.
    for (auto &entry : meta->entries)
        static_cast<PlainArchiveEntry*>(entry.get())->offset -= min_offset;

    return meta;
}

std::unique_ptr<io::File> PakArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> PakArchiveDecoder::get_linked_formats() const
{
    return {"eagls/gr", "eagls/pak-script"};
}

static auto _ = dec::register_decoder<PakArchiveDecoder>("eagls/pak");
