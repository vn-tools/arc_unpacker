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

#include "dec/malie/libu_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"
#include "dec/malie/common/camellia_stream.h"

using namespace au;
using namespace au::dec::malie;

static const auto magic = "LIBU"_b;

namespace
{
    struct CustomArchiveMeta final : dec::ArchiveMeta
    {
        common::LibPlugin plugin;
    };
}

bool LibuArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    for (const auto &plugin : plugin_manager.get_all())
    {
        common::CamelliaStream camellia_stream(input_file.stream, plugin.key);
        const auto maybe_magic = camellia_stream.seek(0).read(4);
        if (maybe_magic == magic)
            return true;
    }
    return false;
}

std::unique_ptr<dec::ArchiveMeta> LibuArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto meta = std::make_unique<CustomArchiveMeta>();

    const auto maybe_magic = input_file.stream.seek(0).read(magic.size());
    meta->plugin = maybe_magic == magic
        ? plugin_manager.get("noop")
        : plugin_manager.get();

    common::CamelliaStream camellia_stream(input_file.stream, meta->plugin.key);
    camellia_stream.seek(magic.size());
    const auto version = camellia_stream.read_le<u32>();
    const auto file_count = camellia_stream.read_le<u32>();
    camellia_stream.skip(4);
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = algo::utf16_to_utf8(camellia_stream.read(68)).str(true);
        entry->size = camellia_stream.read_le<u32>();
        entry->offset = camellia_stream.read_le<u32>();
        camellia_stream.skip(4);
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<io::File> LibuArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto meta = static_cast<const CustomArchiveMeta*>(&m);
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    return std::make_unique<io::File>(
        entry->path,
        std::make_unique<common::CamelliaStream>(
            input_file.stream,
            meta->plugin.key,
            entry->offset,
            entry->size));
}

std::vector<std::string> LibuArchiveDecoder::get_linked_formats() const
{
    return {"malie/libu", "malie/mgf", "malie/dzi"};
}

static auto _ = dec::register_decoder<LibuArchiveDecoder>("malie/libu");
