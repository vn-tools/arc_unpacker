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

#include "dec/cri/hca/permutator.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::cri::hca;

static std::array<u8, 0x100> create_v0_table()
{
    std::array<u8, 0x100> table;
    for (const auto i : algo::range(0x100))
        table[i] = i;
    return table;
}

static std::array<u8, 0x100> create_v1_table()
{
    std::array<u8, 0x100> table;
    table[0] = 0;
    table[0xFF] = 0xFF;
    int v = 0;
    for (const auto i : algo::range(1, 0xFF))
    {
        v = (v * 13 + 11) & 0xFF;
        if (v == 0 || v == 0xFF)
            v = (v * 13 + 11) & 0xFF;
        table[i] = v;
    }
    return table;
}

static std::array<u8, 0x10> create_v56_helper_table(u8 key)
{
    int mul = ((key & 1) << 3) | 5;
    int add = (key & 0xE) | 1;
    key >>= 4;
    std::array<u8, 0x10> table;
    for (const auto i : algo::range(0x10))
    {
        key = (key * mul + add) & 0xF;
        table[i] = key;
    }
    return table;
}

static std::array<u8, 0x100> create_v56_table(u32 key1, u32 key2)
{
    u8 t1[7];
    if (!key1)
        key2--;
    key1--;

    for (const auto i : algo::range(7))
    {
        t1[i] = key1;
        key1 = (key1 >> 8) | (key2 << 24);
        key2 >>= 8;
    }

    const size_t t1a[] = {1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6};
    const size_t t1b[] = {0, 6, 3, 0, 1, 4, 0, 2, 5, 0, 3, 6, 0, 4, 1, 0};
    u8 t2[0x10];
    for (const auto i : algo::range(0x10))
        t2[i] = t1[t1a[i]] ^ (t1b[i] ? t1[t1b[i]] : 0);

    u8 t3[0x100];
    const auto t3t = create_v56_helper_table(t1[0]);
    for (const auto i : algo::range(0x10))
    {
        const auto t32 = create_v56_helper_table(t2[i]);
        for (const auto j : algo::range(0x10))
            t3[i * 0x10 + j] = (t3t[i] << 4) | t32[j];
    }

    std::array<u8, 0x100> table;
    int v = 0;
    for (const auto i : algo::range(0x100))
    {
        v = (v + 0x11) & 0xFF;
        u8 a = t3[v];
        if (a != 0 && a != 0xFF)
            table[i + 1] = a;
    }
    table[0] = 0;
    table[0xFF] = 0xFF;
    return table;
}

struct Permutator::Priv final
{
    std::array<u8, 256> table;
};

Permutator::Permutator(u16 type, const u32 key1, const u32 key2) : p(new Priv)
{
    if (!(key1 | key2))
        type = 0;

    if (type != 0)
        throw err::NotSupportedError("Only cipher type 0 is supported");

    if (type == 0)
        p->table = create_v0_table();
    else if (type == 1)
        p->table = create_v1_table();
    else if (type == 56)
        p->table = create_v56_table(key1, key2);
    else
        throw err::NotSupportedError("Unknown cipher type");
}

Permutator::~Permutator()
{
}

bstr Permutator::permute(const bstr &input)
{
    bstr output(input.size());
    for (const auto i : algo::range(input.size()))
        output[i] = p->table[input[i]];
    return output;
}
