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

#include "dec/cyberworks/dat_archive_decoder.h"
#include "algo/pack/lzss.h"
#include "dec/cyberworks/common/algo.h"
#include "dec/cyberworks/common/plugins.h"
#include "io/memory_byte_stream.h"
#include "virtual_file_system.h"

using namespace au;
using namespace au::dec::cyberworks;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        std::vector<std::unique_ptr<io::File>> data_files;
        DatPlugin plugin;
    };

    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        bstr type;
        u32 data_file_id;
    };
}

static std::vector<std::string> get_data_file_names(
    const io::File &input_file, const DatPlugin &plugin)
{
    const auto toc_file_name = input_file.path.name();
    const auto tmp = plugin.toc_to_data_file_name_map.find(toc_file_name);
    if (tmp == plugin.toc_to_data_file_name_map.end())
    {
        throw err::RecognitionError(
            "Unrecognized TOC file name, cannot proceed.");
    }
    return tmp->second;
}

DatArchiveDecoder::DatArchiveDecoder()
{
    common::register_plugins(plugin_manager);
    add_arg_parser_decorator(
        plugin_manager.create_arg_parser_decorator(
            "Specifies plugin for decoding image files."));
}

bool DatArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(8);
    const auto size_comp = common::read_obfuscated_number(input_file.stream);
    return size_comp == input_file.stream.left();
}

std::unique_ptr<dec::ArchiveMeta> DatArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto size_orig = common::read_obfuscated_number(input_file.stream);
    const auto size_comp = common::read_obfuscated_number(input_file.stream);
    const auto table_comp = input_file.stream.read(size_comp);
    const auto table_orig = algo::pack::lzss_decompress(table_comp, size_orig);
    io::MemoryByteStream table_stream(table_orig);

    auto meta = std::make_unique<CustomArchiveMeta>();
    meta->plugin = plugin_manager.get();
    const auto data_file_names = get_data_file_names(input_file, meta->plugin);
    for (const auto &data_file_name : data_file_names)
        meta->data_files.push_back(
            VirtualFileSystem::get_by_name(data_file_name));

    while (table_stream.left())
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        const auto entry_size = table_stream.read_le<u32>();
        if (entry_size != 23)
            throw err::CorruptDataError("Unexpected entry size");
        const auto file_id = table_stream.read_le<u32>();
        entry->size_orig = table_stream.read_le<u32>();
        entry->size_comp = table_stream.read_le<u32>();
        entry->offset = table_stream.read_le<u32>();
        entry->type = table_stream.read(2);
        const auto unk = table_stream.read_le<u32>(); // FFFFFFFF?
        entry->data_file_id = table_stream.read<u8>();
        if (entry->data_file_id >= meta->data_files.size()
                || !meta->data_files[entry->data_file_id])
        {
            throw err::CorruptDataError("Data file not found.");
        }
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> DatArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    auto data_stream = meta->data_files.at(entry->data_file_id)->stream.clone();
    auto data = data_stream->seek(entry->offset).read(entry->size_comp);
    if (entry->size_orig != entry->size_comp)
        data = algo::pack::lzss_decompress(data, entry->size_orig);

    common::decode_data(logger, entry->type, data, meta->plugin);

    auto ret = std::make_unique<io::File>(entry->path, data);
    ret->guess_extension();
    return ret;
}

static auto _ = dec::register_decoder<DatArchiveDecoder>("cyberworks/dat");
