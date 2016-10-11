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

#include "algo/pack/huffman.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::algo::pack;

static int init_huffman_impl(
    io::BaseBitStream &input_stream, u16 nodes[2][512], int &size)
{
    if (!input_stream.read(1))
        return input_stream.read(8);
    const auto pos = size;
    if (pos > 511)
        return -1;
    size++;
    nodes[0][pos] = init_huffman_impl(input_stream, nodes, size);
    nodes[1][pos] = init_huffman_impl(input_stream, nodes, size);
    return pos;
}

HuffmanTree::HuffmanTree(io::BaseBitStream &input_stream)
{
    size = 256;
    root = init_huffman_impl(input_stream, nodes, size);
}

HuffmanTree::HuffmanTree(const bstr &data)
{
    io::MsbBitStream input_stream(data);
    size = 256;
    root = init_huffman_impl(input_stream, nodes, size);
}

bstr algo::pack::decode_huffman(
    const HuffmanTree &huffman_tree,
    const bstr &input,
    const size_t target_size)
{
    bstr output;
    output.resize(target_size);
    io::MsbBitStream input_stream(input);
    while (output.size() < target_size && input_stream.left())
    {
        auto byte = huffman_tree.root;
        while (byte >= 256 && byte <= 511)
            byte = huffman_tree.nodes[input_stream.read(1)][byte];
        output += static_cast<const u8>(byte);
    }
    return output;
}
