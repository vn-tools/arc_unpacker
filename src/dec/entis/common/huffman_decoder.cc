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

#include "dec/entis/common/huffman_decoder.h"
#include "algo/range.h"
#include "dec/entis/common/gamma_decoder.h"

using namespace au;
using namespace au::dec::entis;
using namespace au::dec::entis::common;

int common::get_huffman_code(io::BaseBitStream &bit_stream, HuffmanTree &tree)
{
    if (tree.escape != HuffmanNodes::Null)
    {
        int entry = HuffmanNodes::Root;
        int child = tree.nodes[HuffmanNodes::Root].code;
        while (!(child & HuffmanFlags::Code))
        {
            if (!bit_stream.left())
                return HuffmanFlags::Escape;
            entry = child + bit_stream.read(1);
            child = tree.nodes[entry].code;
        }

        tree.increase_occurrences(entry);
        int code = child & ~HuffmanFlags::Code;
        if (code != HuffmanFlags::Escape)
            return code;
    }
    if (!bit_stream.left())
        return HuffmanFlags::Escape;
    int code = bit_stream.read(8);
    tree.add_new_entry(code);
    return code;
}

int common::get_huffman_size(io::BaseBitStream &bit_stream, HuffmanTree &tree)
{
    if (tree.escape != HuffmanNodes::Null)
    {
        int entry = HuffmanNodes::Root;
        int child = tree.nodes[HuffmanNodes::Root].code;
        do
        {
            if (!bit_stream.left())
                return HuffmanFlags::Escape;
            entry = child + bit_stream.read(1);
            child = tree.nodes[entry].code;
        }
        while (!(child & HuffmanFlags::Code));

        tree.increase_occurrences(entry);
        int code = child & ~HuffmanFlags::Code;
        if (code != HuffmanFlags::Escape)
            return code;
    }
    int code = get_gamma_code(bit_stream);
    if (code == -1)
        return HuffmanFlags::Escape;
    tree.add_new_entry(code);
    return code;
}

struct HuffmanDecoder::Priv final
{
    std::vector<std::shared_ptr<HuffmanTree>> huffman_trees;
    std::shared_ptr<HuffmanTree> last_huffman_tree;
    size_t available_size;
};

HuffmanDecoder::HuffmanDecoder() : p(new Priv())
{
}

HuffmanDecoder::~HuffmanDecoder()
{
}

void HuffmanDecoder::reset()
{
    p->huffman_trees.clear();
    for (const auto i : algo::range(0x101))
        p->huffman_trees.push_back(std::make_shared<HuffmanTree>());
    p->last_huffman_tree = p->huffman_trees[0];
    p->available_size = 0;
}

void HuffmanDecoder::decode(u8 *output, const size_t output_size)
{
    if (!bit_stream)
        throw std::logic_error("Trying to decode with unitialized input");
    if (!p->huffman_trees.size())
        throw std::logic_error("Trying to decode with unitialized state");

    auto tree = p->last_huffman_tree;

    u8 *output_ptr = output;
    u8 *output_end = output + output_size;

    if (p->available_size > 0)
    {
        auto size = std::min<size_t>(p->available_size, output_size);
        p->available_size -= size;
        while (size-- && output_ptr < output_end)
            *output_ptr++ = 0;
    }

    while (output_ptr < output_end)
    {
        int symbol = get_huffman_code(*bit_stream, *tree);
        if (symbol == HuffmanFlags::Escape)
            break;
        *output_ptr++ = symbol;

        if (!symbol)
        {
            int size = get_huffman_size(*bit_stream, *p->huffman_trees[0x100]);
            if (size == HuffmanFlags::Escape)
                break;
            if (--size)
            {
                p->available_size = size;
                if (output_ptr + size > output_end)
                    size = output_end - output_ptr;
                p->available_size -= size;
                while (size-- && output_ptr < output_end)
                    *output_ptr++ = 0;
            }
        }
        tree = p->huffman_trees[symbol & 0xFF];
    }
    p->last_huffman_tree = tree;
}
