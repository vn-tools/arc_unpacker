#include "fmt/bgi/cbg/cbg_common.h"
#include "err.h"
#include "fmt/bgi/common.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::bgi;
using namespace au::fmt::bgi::cbg;

bstr cbg::read_decrypted_data(io::IO &io)
{
    u32 key = io.read_u32_le();
    u32 data_size = io.read_u32_le();

    u8 expected_sum = io.read_u8();
    u8 expected_xor = io.read_u8();
    io.skip(2);

    bstr data = io.read(data_size);
    u8 *data_ptr = data.get<u8>();

    u8 actual_sum = 0;
    u8 actual_xor = 0;
    for (auto i : util::range(data.size()))
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

u32 cbg::read_variable_data(io::IO &input_io)
{
    u8 current;
    u32 result = 0;
    u32 shift = 0;
    do
    {
        current = input_io.read_u8();
        result |= (current & 0x7F) << shift;
        shift += 7;
    } while (current & 0x80);
    return result;
}

FreqTable cbg::read_freq_table(io::IO &input_io, size_t tree_size)
{
    FreqTable freq_table(tree_size);
    for (auto i : util::range(tree_size))
        freq_table[i] = cbg::read_variable_data(input_io);
    return freq_table;
}

NodeInfo &Tree::operator[](size_t index)
{
    return *nodes[index];
}

u32 Tree::get_leaf(io::BitReader &bit_reader) const
{
    u32 node = nodes.size() - 1;
    while (node >= size)
        node = nodes.at(node)->children[bit_reader.get(1)];
    return node;
}

Tree cbg::build_tree(const FreqTable &freq_table, bool greedy)
{
    Tree tree;
    tree.size = freq_table.size();
    u32 freq_sum = 0;
    for (auto i : util::range(tree.size))
    {
        std::shared_ptr<NodeInfo> node(new NodeInfo);
        node->frequency = freq_table[i];
        node->valid = freq_table[i] > 0;
        node->children[0] = i;
        node->children[1] = i;
        freq_sum += freq_table[i];
        tree.nodes.push_back(std::move(node));
    }

    for (auto level : util::range(tree.size))
    {
        u32 freq = 0;
        u32 children[2];
        for (auto j : util::range(2))
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

            for (auto k : util::range(greedy ? j + 1 : 0, tree.nodes.size()))
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
        std::shared_ptr<NodeInfo> node(new NodeInfo);
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
