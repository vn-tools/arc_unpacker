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

#include "dec/bgi/cbg/cbg_common.h"
#include "algo/range.h"
#include "dec/bgi/common.h"
#include "err.h"

using namespace au;
using namespace au::dec::bgi;
using namespace au::dec::bgi::cbg;

bstr cbg::read_decrypted_data(io::BaseByteStream &input_stream)
{
    u32 key = input_stream.read_le<u32>();
    const u32 data_size = input_stream.read_le<u32>();

    u8 expected_sum = input_stream.read<u8>();
    u8 expected_xor = input_stream.read<u8>();
    input_stream.skip(2);

    auto data = input_stream.read(data_size);
    auto data_ptr = data.get<u8>();

    u8 actual_sum = 0;
    u8 actual_xor = 0;
    for (const auto i : algo::range(data.size()))
    {
        *data_ptr -= get_and_update_key(key);
        actual_sum += *data_ptr;
        actual_xor ^= *data_ptr;
        data_ptr++;
    }

    if (actual_sum != expected_sum || actual_xor != expected_xor)
        throw err::CorruptDataError("Checksum test failed");
    return data;
}

u32 cbg::read_variable_data(io::BaseByteStream &input_stream)
{
    u8 current;
    u32 result = 0;
    u32 shift = 0;
    do
    {
        current = input_stream.read<u8>();
        result |= (current & 0x7F) << shift;
        shift += 7;
    } while (current & 0x80);
    return result;
}

FreqTable cbg::read_freq_table(
    io::BaseByteStream &input_stream, const size_t tree_size)
{
    FreqTable freq_table(tree_size);
    for (const auto i : algo::range(tree_size))
        freq_table[i] = cbg::read_variable_data(input_stream);
    return freq_table;
}

NodeInfo &Tree::operator[](size_t index)
{
    return *nodes[index];
}

u32 Tree::get_leaf(io::BaseBitStream &bit_stream) const
{
    u32 node = nodes.size() - 1;
    while (node >= size)
        node = nodes.at(node)->children[bit_stream.read(1)];
    return node;
}

Tree cbg::build_tree(const FreqTable &freq_table, bool greedy)
{
    Tree tree;
    tree.size = freq_table.size();
    u32 freq_sum = 0;
    for (const auto i : algo::range(tree.size))
    {
        auto node = std::make_shared<NodeInfo>();
        node->frequency = freq_table[i];
        node->valid = freq_table[i] > 0;
        node->children[0] = i;
        node->children[1] = i;
        freq_sum += freq_table[i];
        tree.nodes.push_back(std::move(node));
    }

    for (const auto level : algo::range(tree.size))
    {
        u32 freq = 0;
        u32 children[2];
        for (const auto j : algo::range(2))
        {
            u32 min = 0xFFFFFFFF;
            children[j] = 0xFFFFFFFF;

            if (greedy)
            {
                u32 tmp = 0;
                while (tmp < tree.nodes.size() && !tree[tmp].valid)
                    tmp++;
                if (tmp < tree.nodes.size())
                {
                    children[j] = tmp;
                    min = tree[tmp].frequency;
                }
            }

            for (const auto k : algo::range(
                greedy ? j + 1 : 0, tree.nodes.size()))
            {
                if (tree[k].valid && tree[k].frequency < min)
                {
                    min = tree[k].frequency;
                    children[j] = k;
                }
            }
            if (children[j] == 0xFFFFFFFF)
                continue;
            if (!tree[children[j]].valid)
                throw std::logic_error("Invalid Huffman node");
            tree[children[j]].valid = false;
            freq += tree[children[j]].frequency;
        }
        auto node = std::make_shared<NodeInfo>();
        node->valid = true;
        node->frequency = freq;
        node->children[0] = children[0];
        node->children[1] = children[1];
        tree.nodes.push_back(std::move(node));

        if (freq >= freq_sum)
            break;
    }
    return tree;
}
