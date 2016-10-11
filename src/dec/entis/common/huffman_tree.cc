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

#include "dec/entis/common/huffman_tree.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::entis::common;

HuffmanNode::HuffmanNode()
{
    weight = 0;
    parent = 0;
    code = 0;
}

HuffmanTree::HuffmanTree()
{
    for (const auto i : algo::range(0x100))
        sym_lookup[i] = HuffmanNodes::Null;
    escape = HuffmanNodes::Null;
    tree_pointer = HuffmanNodes::Root;
    nodes[HuffmanNodes::Root].weight = 0;
    nodes[HuffmanNodes::Root].parent = HuffmanNodes::Null;
    nodes[HuffmanNodes::Root].code = HuffmanNodes::Null;
}

void HuffmanTree::increase_occurrences(int entry)
{
    nodes[entry].weight ++;
    normalize(entry);
    if (nodes[HuffmanNodes::Root].weight >= 0x4000)
        half_and_rebuild();
}

void HuffmanTree::normalize(int entry)
{
    while (entry < HuffmanNodes::Root)
    {
        int swap = entry + 1;
        u16 weight = nodes[entry].weight;
        while (swap < HuffmanNodes::Root)
        {
            if (nodes[swap].weight >= weight)
                break;
            ++swap;
        }
        if (entry == --swap)
        {
            entry = nodes[entry].parent;
            recount_occurrences(entry);
            continue;
        }
        int child, code;
        if (!(nodes[entry].code & HuffmanFlags::Code))
        {
            child = nodes[entry].code;
            nodes[child].parent = swap;
            nodes[child + 1].parent = swap;
        }
        else
        {
            code = nodes[entry].code & ~HuffmanFlags::Code;
            if (code != HuffmanFlags::Escape)
                sym_lookup[code & 0xFF] = swap;
            else
                escape = swap;
        }
        if (!(nodes[swap].code & HuffmanFlags::Code))
        {
            int child = nodes[swap].code;
            nodes[child].parent = entry;
            nodes[child+1].parent = entry;
        }
        else
        {
            int code = nodes[swap].code & ~HuffmanFlags::Code;
            if (code != HuffmanFlags::Escape)
                sym_lookup[code & 0xFF] = entry;
            else
                escape = entry;
        }
        HuffmanNode node;
        u16 entry_parent = nodes[entry].parent;
        u16 swap_parent = nodes[swap].parent;
        node = nodes[swap];
        nodes[swap] = nodes[entry];
        nodes[entry] = node;
        nodes[swap].parent = swap_parent;
        nodes[entry].parent = entry_parent;
        recount_occurrences(swap_parent);
        entry = swap_parent;
    }
}

void HuffmanTree::recount_occurrences(int parent)
{
    int child = nodes[parent].code;
    nodes[parent].weight = nodes[child].weight + nodes[child + 1].weight;
}

void HuffmanTree::add_new_entry(int new_code)
{
    if (tree_pointer > 0)
    {
        tree_pointer -= 2;
        int i = tree_pointer;
        HuffmanNode &new_node = nodes[i];
        new_node.weight = 1;
        new_node.code = HuffmanFlags::Code | new_code;
        sym_lookup[new_code & 0xFF] = i;
        HuffmanNode &root_node = nodes[HuffmanNodes::Root];

        if (root_node.code != HuffmanNodes::Null)
        {
            HuffmanNode &parent_node = nodes[i + 2];
            HuffmanNode &child_node = nodes[i + 1];
            nodes[i + 1] = nodes[i + 2];
            if (child_node.code & HuffmanFlags::Code)
            {
                int code = child_node.code & ~HuffmanFlags::Code;
                if (code != HuffmanFlags::Escape)
                    sym_lookup[code & 0xFF] = i + 1;
                else
                    escape = i + 1;
            }
            parent_node.weight = new_node.weight + child_node.weight;
            parent_node.parent = child_node.parent;
            parent_node.code = i;
            new_node.parent = child_node.parent = i + 2;
            normalize(i + 2);
        }
        else
        {
            new_node.parent = HuffmanNodes::Root;
            HuffmanNode &escape_node = nodes[escape = i + 1];
            escape_node.weight = 1;
            escape_node.parent = HuffmanNodes::Root;
            escape_node.code = HuffmanFlags::Code | HuffmanFlags::Escape;
            root_node.weight = 2;
            root_node.code = i;
        }
    }
    else
    {
        int i = tree_pointer;
        HuffmanNode *node = &nodes[i];
        if (node->code == (HuffmanFlags::Code | HuffmanFlags::Escape))
            node = &nodes[i + 1];
        node->code = HuffmanFlags::Code | new_code;
    }
}

void HuffmanTree::half_and_rebuild()
{
    int i;
    int next_entry = HuffmanNodes::Root;
    for (i = HuffmanNodes::Root - 1; i >= tree_pointer; i--)
    {
        if (nodes[i].code & HuffmanFlags::Code)
        {
            nodes[i].weight = (nodes[i].weight + 1) >> 1;
            nodes[next_entry--] = nodes[i];
        }
    }
    ++next_entry;
    int child;
    i = tree_pointer;

    while (true)
    {
        nodes[i] = nodes[next_entry];
        nodes[i + 1] = nodes[next_entry + 1];
        next_entry += 2;
        HuffmanNode *child1_node = &nodes[i];
        HuffmanNode *child2_node = &nodes[i + 1];

        if (!(child1_node->code & HuffmanFlags::Code))
        {
            child = child1_node->code;
            nodes[child].parent = i;
            nodes[child + 1].parent = i;
        }
        else
        {
            int code = child1_node->code & ~HuffmanFlags::Code;
            if (code == HuffmanFlags::Escape)
                escape = i;
            else
                sym_lookup[code & 0xFF] = i;
        }

        if (!(child2_node->code & HuffmanFlags::Code))
        {
            child = child2_node->code;
            nodes[child].parent = i + 1;
            nodes[child + 1].parent = i + 1;
        }
        else
        {
            int code = child2_node->code & ~HuffmanFlags::Code;
            if (code == HuffmanFlags::Escape)
                escape = i + 1;
            else
                sym_lookup[code & 0xFF] = i + 1;
        }

        u16 weight = child1_node->weight + child2_node->weight;
        if (next_entry <= HuffmanNodes::Root)
        {
            int j = next_entry;
            while (true)
            {
                if (weight <= nodes[j].weight)
                {
                    nodes[j - 1].weight = weight;
                    nodes[j - 1].code = i;
                    break;
                }
                nodes[j - 1] = nodes[j];
                if (++j > HuffmanNodes::Root)
                {
                    nodes[HuffmanNodes::Root].weight = weight;
                    nodes[HuffmanNodes::Root].code = i;
                    break;
                }
            }
            --next_entry;
        }
        else
        {
            nodes[HuffmanNodes::Root].weight = weight;
            nodes[HuffmanNodes::Root].parent = HuffmanNodes::Null;
            nodes[HuffmanNodes::Root].code = i;
            child1_node->parent = HuffmanNodes::Root;
            child2_node->parent = HuffmanNodes::Root;
            break;
        }

        i += 2;
    }
}
