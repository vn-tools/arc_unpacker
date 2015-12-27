#pragma once

#include "io/bit_reader.h"

namespace au {
namespace algo {
namespace pack {

    struct HuffmanTree final
    {
        HuffmanTree(const bstr &data);
        HuffmanTree(io::IBitReader &bit_reader);

        int size;
        u16 root;
        u16 nodes[2][512];
    };

    bstr decode_huffman(
        const HuffmanTree &huffman_tree,
        const bstr &input,
        const size_t target_size);

} } }
