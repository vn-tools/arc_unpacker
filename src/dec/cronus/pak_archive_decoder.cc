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

#include "dec/cronus/pak_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "dec/cronus/common.h"
#include "err.h"
#include "io/memory_byte_stream.h"
#include "plugin_manager.h"

using namespace au;
using namespace au::dec::cronus;

static const bstr magic2 = "CHERRY PACK 2.0\x00"_b;
static const bstr magic3 = "CHERRY PACK 3.0\x00"_b;

namespace
{
    struct Plugin final
    {
        u32 key1;
        u32 key2;
    };
}

static std::unique_ptr<dec::ArchiveMeta> read_meta(
    io::File &input_file, const Plugin &plugin, bool encrypted)
{
    const auto file_count = input_file.stream.read_le<u32>() ^ plugin.key1;
    const auto file_data_start = input_file.stream.read_le<u32>() ^ plugin.key2;
    if (file_data_start > input_file.stream.size())
        return nullptr;

    const auto table_size_orig = file_count * 24;
    const auto table_size_comp = file_data_start - input_file.stream.pos();
    auto table_data = input_file.stream.read(table_size_comp);
    if (encrypted)
    {
        const auto key = get_delta_key("CHERRYSOFT"_b);
        delta_decrypt(table_data, key);
        table_data = algo::pack::lzss_decompress(table_data, table_size_orig);
    }
    io::MemoryByteStream table_stream(table_data);

    auto meta = std::make_unique<dec::ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<dec::PlainArchiveEntry>();
        entry->path = table_stream.read_to_zero(16).str();
        entry->offset = table_stream.read_le<u32>() + file_data_start;
        entry->size = table_stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

struct PakArchiveDecoder::Priv final
{
    PluginManager<Plugin> plugin_manager;
};

PakArchiveDecoder::PakArchiveDecoder() : p(new Priv)
{
    p->plugin_manager.add("default", "Unencrypted games", {0, 0});
    p->plugin_manager.add("sweet", "Sweet Pleasure", {0xBC138744, 0x64E0BA23});
}

PakArchiveDecoder::~PakArchiveDecoder()
{
}

bool PakArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic2.size()) == magic2)
        return true;
    input_file.stream.seek(0);
    return input_file.stream.read(magic3.size()) == magic3;
}

std::unique_ptr<dec::ArchiveMeta> PakArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    if (input_file.stream.read(magic2.size()) != magic2)
        input_file.stream.seek(magic3.size());
    const auto encrypted = input_file.stream.read_le<u32>() > 0;
    const auto pos = input_file.stream.pos();
    for (const auto &plugin : p->plugin_manager.get_all())
    {
        input_file.stream.seek(pos);
        try
        {
            auto meta = ::read_meta(input_file, plugin, encrypted);
            if (meta && meta->entries.size())
                return meta;
        }
        catch (...)
        {
            continue;
        }
    }
    throw err::RecognitionError("Unknown encryption scheme");
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
    return {"cronus/grp"};
}

static auto _ = dec::register_decoder<PakArchiveDecoder>("cronus/pak");
