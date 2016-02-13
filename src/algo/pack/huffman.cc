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
