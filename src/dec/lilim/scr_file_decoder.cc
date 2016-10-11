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

#include "dec/lilim/scr_file_decoder.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::lilim;

static int init_huffman(
    io::BaseBitStream &bit_stream, u16 nodes[2][512], int &size)
{
    if (!bit_stream.read(1))
        return bit_stream.read(8);
    const auto pos = size;
    if (pos > 511)
        return -1;
    size++;
    nodes[0][pos] = init_huffman(bit_stream, nodes, size);
    nodes[1][pos] = init_huffman(bit_stream, nodes, size);
    return pos;
}

static bstr decode_huffman(const bstr &input, const size_t target_size)
{
    bstr output;
    output.reserve(input.size() * 2);
    io::MsbBitStream bit_stream(input);
    u16 nodes[2][512];
    auto size = 256;
    auto root = init_huffman(bit_stream, nodes, size);
    while (output.size() < target_size && bit_stream.left())
    {
        auto byte = root;
        while (byte >= 256 && byte <= 511)
            byte = nodes[bit_stream.read(1)][byte];
        output += static_cast<const u8>(byte);
    }
    return output;
}

bool ScrFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("scr");
}

std::unique_ptr<io::File> ScrFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto size_orig = input_file.stream.read_le<u32>();
    auto data = input_file.stream.read_to_eof();
    data = decode_huffman(data, size_orig);
    auto output_file = std::make_unique<io::File>(input_file.path, data);
    output_file->path.change_extension("txt");
    return output_file;
}

static auto _ = dec::register_decoder<ScrFileDecoder>("lilim/scr");
