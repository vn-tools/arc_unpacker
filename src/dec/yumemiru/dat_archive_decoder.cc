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

#include "dec/yumemiru/dat_archive_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::yumemiru;

static const bstr magic1 = "yanepkDx"_b;
static const bstr magic2 = "PackDat3"_b;

bool DatArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    if (input_file.stream.read(magic1.size()) == magic1)
        return true;
    input_file.stream.seek(0);
    return input_file.stream.read(magic2.size()) == magic2;
}

std::unique_ptr<dec::ArchiveMeta> DatArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(8);
    const auto file_count = input_file.stream.read_le<u32>();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CompressedArchiveEntry>();
        entry->path = input_file.stream.read_to_zero(0x100).str();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size_orig = input_file.stream.read_le<u32>();
        entry->size_comp = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> DatArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CompressedArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);
    if (entry->size_orig != entry->size_comp)
        throw err::NotSupportedError("Compressed archives are not supported");
    const auto data = input_file.stream.read(entry->size_comp);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> DatArchiveDecoder::get_linked_formats() const
{
    return {"yumemiru/epf"};
}

static auto _ = dec::register_decoder<DatArchiveDecoder>("yumemiru/dat");
