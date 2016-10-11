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

#include "dec/kaguya/wfl_archive_decoder.h"
#include "algo/binary.h"
#include "algo/locale.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::kaguya;

static const auto magic = "WFL1"_b;

namespace
{
    struct CustomArchiveEntry final : dec::CompressedArchiveEntry
    {
        u16 type;
    };
}

static bstr decompress(const bstr &input, const size_t size_orig)
{
    algo::pack::BitwiseLzssSettings settings;
    settings.min_match_size = 2;
    settings.position_bits = 12;
    settings.size_bits = 4;
    settings.initial_dictionary_pos = 1;
    return algo::pack::lzss_decompress(input, size_orig, settings);
}

bool WflArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> WflArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    auto meta = std::make_unique<ArchiveMeta>();
    while (input_file.stream.left() >= 4)
    {
        const auto name_size = input_file.stream.read_le<u32>();
        if (!name_size)
            break;
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = algo::sjis_to_utf8(algo::unxor(
            input_file.stream.read(name_size), 0xFF).str()).str(true);
        entry->type = input_file.stream.read_le<u16>();
        entry->size_comp = input_file.stream.read_le<u32>();
        entry->size_orig = entry->type == 1
            ? input_file.stream.read_le<u32>()
            : entry->size_comp;
        entry->offset = input_file.stream.pos();
        input_file.stream.skip(entry->size_comp);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> WflArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    const auto data_comp
        = input_file.stream.seek(entry->offset).read(entry->size_comp);
    const auto data_orig = entry->type == 1
        ? decompress(data_comp, entry->size_orig)
        : data_comp;
    auto output_file = std::make_unique<io::File>(entry->path, data_orig);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> WflArchiveDecoder::get_linked_formats() const
{
    return {"kaguya/ap", "kaguya/ao", "kaguya/aps3", "microsoft/bmp"};
}

static auto _ = dec::register_decoder<WflArchiveDecoder>("kaguya/wfl");
