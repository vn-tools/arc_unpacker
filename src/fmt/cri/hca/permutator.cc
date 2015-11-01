#include "fmt/cri/hca/permutator.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::cri::hca;

static std::array<u8, 0x100> create_v0_table()
{
    std::array<u8, 0x100> table;
    for (const auto i : util::range(0x100))
        table[i] = i;
    return table;
}

static std::array<u8, 0x100> create_v1_table()
{
    std::array<u8, 0x100> table;
    table[0] = 0;
    table[0xFF] = 0xFF;
    int v = 0;
    for (const auto i : util::range(1, 0xFF))
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
    for (const auto i : util::range(0x10))
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

    for (const auto i : util::range(7))
    {
        t1[i] = key1;
        key1 = (key1 >> 8) | (key2 << 24);
        key2 >>= 8;
    }

    u8 t2[0x10] =
    {
        // gcc persistently complains without these casts
        static_cast<u8>(t1[1]),
        static_cast<u8>(t1[1] ^ t1[6]),
        static_cast<u8>(t1[2] ^ t1[3]),
        static_cast<u8>(t1[2]),
        static_cast<u8>(t1[2] ^ t1[1]),
        static_cast<u8>(t1[3] ^ t1[4]),
        static_cast<u8>(t1[3]),
        static_cast<u8>(t1[3] ^ t1[2]),
        static_cast<u8>(t1[4] ^ t1[5]),
        static_cast<u8>(t1[4]),
        static_cast<u8>(t1[4] ^ t1[3]),
        static_cast<u8>(t1[5] ^ t1[6]),
        static_cast<u8>(t1[5]),
        static_cast<u8>(t1[5] ^ t1[4]),
        static_cast<u8>(t1[6] ^ t1[1]),
        static_cast<u8>(t1[6]),
    };

    u8 t3[0x100];
    const auto t31 = create_v56_helper_table(t1[0]);
    for (const auto i : util::range(0x10))
    {
        const auto t32 = create_v56_helper_table(t2[i]);
        for (const auto j : util::range(0x10))
            t3[i * 0x10 + j] = (t31[i] << 4) | t32[j];
    }

    std::array<u8, 0x100> table;
    int v = 0;
    for (const auto i : util::range(0x100))
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
    for (const auto i : util::range(input.size()))
        output[i] = p->table[input[i]];
    return output;
}
