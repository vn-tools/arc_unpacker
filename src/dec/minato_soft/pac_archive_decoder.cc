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

#include "dec/minato_soft/pac_archive_decoder.h"
#include "algo/locale.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::minato_soft;

static const bstr magic = "PAC\x00"_b;

static int init_huffman(
    io::BaseBitStream &bit_stream, u16 nodes[2][512], int &pos)
{
    if (bit_stream.read(1))
    {
        const auto old_pos = pos;
        pos++;
        if (old_pos < 511)
        {
            nodes[0][old_pos] = init_huffman(bit_stream, nodes, pos);
            nodes[1][old_pos] = init_huffman(bit_stream, nodes, pos);
            return old_pos;
        }
        return -1;
    }
    return bit_stream.read(8);
}

static bstr decompress_table(const bstr &input, size_t output_size)
{
    bstr output(output_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();
    io::MsbBitStream bit_stream(input);

    u16 nodes[2][512];
    auto pos = 256;
    const auto initial_pos = init_huffman(bit_stream, nodes, pos);

    while (output_ptr < output_end)
    {
        auto pos = initial_pos;
        while (pos >= 256 && pos <= 511)
            pos = nodes[bit_stream.read(1)][pos];

        *output_ptr++ = pos;
    }
    return output;
}

bool PacArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> PacArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    input_file.stream.seek(input_file.stream.size() - 4);
    const auto size_comp = input_file.stream.read_le<u32>();
    const auto size_orig = file_count * 76;

    input_file.stream.seek(input_file.stream.size() - 4 - size_comp);
    auto compressed = input_file.stream.read(size_comp);
    for (const auto i : algo::range(compressed.size()))
        compressed.get<u8>()[i] ^= 0xFF;

    io::MemoryByteStream table_stream(decompress_table(compressed, size_orig));
    table_stream.seek(0);

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CompressedArchiveEntry>();
        entry->path = algo::sjis_to_utf8(table_stream.read_to_zero(0x40)).str();
        entry->offset = table_stream.read_le<u32>();
        entry->size_orig = table_stream.read_le<u32>();
        entry->size_comp = table_stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> PacArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CompressedArchiveEntry*>(&e);
    auto data = input_file.stream.seek(entry->offset).read(entry->size_comp);
    if (entry->size_orig != entry->size_comp)
        data = algo::pack::zlib_inflate(data);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> PacArchiveDecoder::get_linked_formats() const
{
    return {"minato-soft/fil"};
}

static auto _ = dec::register_decoder<PacArchiveDecoder>("minato-soft/pac");
