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

#include "dec/leaf/single_letter_group/a_archive_decoder.h"
#include "algo/locale.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "\x1E\xAF"_b; // LEAF in hexspeak

namespace
{
    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        u8 flags;
    };
}

static bstr decrypt_1(const bstr &input)
{
    throw err::NotSupportedError("Type 1 encryption is not supported");
}

static bstr decrypt_2(const bstr &input)
{
    throw err::NotSupportedError("Type 2 encryption is not supported");
}

static bstr decrypt_3(const bstr &input, const u8 key)
{
    if (input.size() < 0x20)
        return input;

    bstr output(input);

    io::MemoryByteStream input_stream(input);
    input_stream.seek(0);
    const auto width = input_stream.read_le<u32>();
    const auto height = input_stream.read_le<u32>();
    const auto size = width * height;
    if (32 + size * 4 > input_stream.size())
        return input;

    s8 acc[3] = {0, 0, 0};
    size_t output_pos = 32;
    for (const auto i : algo::range(size))
    {
        s8 *output_ptr = output.get<s8>() + output_pos;
        const auto tmp = output_ptr[3];
        acc[0] += tmp + output_ptr[0] - key;
        acc[1] += tmp + output_ptr[1] - key;
        acc[2] += tmp + output_ptr[2] - key;
        output_ptr[0] = acc[0];
        output_ptr[1] = acc[1];
        output_ptr[2] = acc[2];
        output_ptr[3] = 0;
        output_pos += 4;
    }

    return output;
}

bool AArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> AArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u16>();
    const auto offset_to_data = input_file.stream.pos() + 32 * file_count;
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = algo::sjis_to_utf8(
            input_file.stream.read_to_zero(23)).str();
        entry->flags = input_file.stream.read<u8>();
        entry->size = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.read_le<u32>() + offset_to_data;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> AArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    input_file.stream.seek(entry->offset);

    bstr data;
    if (entry->flags)
    {
        const auto size_orig = input_file.stream.read_le<u32>();
        data = input_file.stream.read(entry->size-4);
        data = algo::pack::lzss_decompress(data, size_orig);

        // this "encryption" apparently concerns only gfx
        if (entry->flags == 3)
            data = decrypt_1(data);
        else if (entry->flags == 5)
            data = decrypt_2(data);
        else if (entry->flags >= 0x7F && entry->flags <= 0x89)
            data = decrypt_3(data, entry->flags & 0x0F);
    }
    else
    {
        data = input_file.stream.read(entry->size);
    }
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> AArchiveDecoder::get_linked_formats() const
{
    return {"leaf/w", "leaf/g", "leaf/px"};
}

static auto _ = dec::register_decoder<AArchiveDecoder>("leaf/a");
