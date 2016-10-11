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

#include "dec/propeller/mgr_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::propeller;

namespace
{
    struct CustomArchiveEntry final : dec::ArchiveEntry
    {
        uoff_t offset;
    };
}

static bstr decompress(const bstr &input, size_t size_orig)
{
    const u8 *input_ptr = input.get<const u8>();
    const u8 *input_end = input_ptr + input.size();

    bstr output(size_orig);
    u8 *output_ptr = output.get<u8>();
    u8 *output_end = output_ptr + size_orig;

    while (output_ptr < output_end)
    {
        const auto c = *input_ptr++;

        if (c < 0x20)
        {
            size_t repetitions = c + 1;
            while (repetitions--)
                *output_ptr++ = *input_ptr++;
        }
        else
        {
            size_t repetitions = c >> 5;
            if (repetitions == 7)
                repetitions += *input_ptr++;
            repetitions += 2;

            auto look_behind = ((c & 0x1F) << 8) + 1;
            look_behind += *input_ptr++;

            u8 *source = output_ptr - look_behind;
            while (repetitions--)
                *output_ptr++ = *source++;
        }
    }

    return output;
}

bool MgrArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("mgr");
}

std::unique_ptr<dec::ArchiveMeta> MgrArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto entry_count = input_file.stream.read_le<u16>();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(entry_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->offset = entry_count == 1
            ? input_file.stream.pos()
            : input_file.stream.read_le<u32>();
        entry->path = algo::format("%d.bmp", i);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> MgrArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto size_comp = input_file.stream.read_le<u32>();

    auto data = input_file.stream.read(size_comp);
    data = decompress(data, size_orig);
    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<MgrArchiveDecoder>("propeller/mgr");
