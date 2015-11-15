#include "util/pack/huffman.h"

using namespace au;
using namespace au::util::pack;

static int init_huffman_impl(
    io::BitReader &bit_reader, u16 nodes[2][512], int &size)
{
    if (!bit_reader.get(1))
        return bit_reader.get(8);
    const auto pos = size;
    if (pos > 511)
        return -1;
    size++;
    nodes[0][pos] = init_huffman_impl(bit_reader, nodes, size);
    nodes[1][pos] = init_huffman_impl(bit_reader, nodes, size);
    return pos;
}

HuffmanTree::HuffmanTree(io::BitReader &bit_reader)
{
    size = 256;
    root = init_huffman_impl(bit_reader, nodes, size);
}

HuffmanTree::HuffmanTree(const bstr &data)
{
    io::BitReader bit_reader(data);
    size = 256;
    root = init_huffman_impl(bit_reader, nodes, size);
}

bstr util::pack::decode_huffman(
    const HuffmanTree &huffman_tree,
    const bstr &input,
    const size_t target_size)
{
    bstr output;
    output.resize(target_size);
    io::BitReader bit_reader(input);
    u16 nodes[2][512];
    while (output.size() < target_size && !bit_reader.eof())
    {
        auto byte = huffman_tree.root;
        while (byte >= 256 && byte <= 511)
            byte = huffman_tree.nodes[bit_reader.get(1)][byte];
        output += static_cast<const u8>(byte);
    }
    return output;
}
