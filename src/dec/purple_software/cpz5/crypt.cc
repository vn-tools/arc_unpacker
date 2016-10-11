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

#include "dec/purple_software/cpz5/crypt.h"
#include "algo/binary.h"
#include "algo/crypt/md5.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::purple_software;
using namespace au::dec::purple_software::cpz5;

static std::array<u8, 256> get_table_for_crypt_2_and_3(
    const Plugin &plugin, u32 key, const u32 seed)
{
    std::array<u8, 256> table;
    for (const auto i : algo::range(256))
        table[i] = i;
    for (const auto i : algo::range(256))
    {
        std::swap(table[(key >> 16) & 0xFF], table[key & 0xFF]);
        std::swap(table[(key >> 8) & 0xFF], table[(key >> 24) & 0xFF]);
        key = seed + algo::rotr<u32>(key, 2) * plugin.crypt_23_mul;
    }
    return table;
}

std::array<u32, 4> cpz5::get_hash(
    const Plugin &plugin, const std::array<u32, 4> &input_dwords)
{
    io::MemoryByteStream tmp_stream;
    for (const auto &dword : input_dwords)
        tmp_stream.write_le<u32>(dword);

    const auto hash_bytes = algo::crypt::md5(
        tmp_stream.seek(0).read_to_eof(), plugin.hash_iv);

    tmp_stream.seek(0).write(hash_bytes).seek(0);
    std::array<u32, 4> hash_dwords;
    for (const auto i : algo::range(4))
        hash_dwords[i] = tmp_stream.read_le<u32>();

    std::array<u32, 4> result;
    for (const auto i : algo::range(4))
    {
        result[i] = hash_dwords[plugin.hash_permutation[i]];
        result[i] ^= plugin.hash_xor[i];
        result[i] += plugin.hash_add[i];
    }

    return result;
}

void cpz5::decrypt_1a(
    algo::ptr<u8> target, const Plugin &plugin, const u32 key)
{
    std::array<u32, 24> table;
    const auto limit = std::min(table.size(), plugin.secret.size() / 4);
    for (const auto i : algo::range(limit))
        table[i] = plugin.secret.get<u32>()[i] - key;

    size_t shift = key;
    for (const auto i : algo::range(3))
        shift = (shift >> 8) ^ key;
    shift = ((shift ^ 0xFB) & 0x0F) + 7;

    size_t table_pos = 5;

    auto target_u32 = reinterpret_cast<u32*>(&target[0]);
    for (const auto i : algo::range(target.size() >> 2))
    {
        const auto tmp1 = table[table_pos++] ^ *target_u32;
        const auto tmp2 = tmp1 + plugin.crypt_1a.add1;
        *target_u32++ = algo::rotr<u32>(tmp2, shift) + plugin.crypt_1a.add2;
        table_pos %= table.size();
    }

    auto target_u8 = reinterpret_cast<u8*>(target_u32);
    for (const auto i : algo::range(target.size() & 3))
    {
        const auto tmp1 = table[table_pos++] >> (4 * (3 - i));
        const auto tmp2 = tmp1 ^ *target_u8;
        *target_u8 = tmp2 - plugin.crypt_1a.tail_sub;
        target_u8++;
        table_pos %= table.size();
    }
}

void cpz5::decrypt_1b(
    algo::ptr<u8> target, const u32 key, const std::array<u32, 4> &hash)
{
    static const std::array<u32, 4> addends = {0x76A3BF29, 0, 0x10000000, 0};
    std::array<u32, 4> table;
    for (const auto i : algo::range(4))
        table[i] = hash[i] ^ (key + addends[i]);

    size_t table_pos = 0;

    auto target_u32 = reinterpret_cast<u32*>(&target[0]);
    u32 seed = 0x76548AEF;
    for (const auto i : algo::range(target.size() >> 2))
    {
        const u32 tmp = (*target_u32 ^ table[table_pos++]) - 0x4A91C262;
        *target_u32++ = algo::rotl<u32>(tmp, 3) - seed;
        table_pos %= table.size();
        seed += 0x10FB562A;
    }

    auto target_u8 = reinterpret_cast<u8*>(target_u32);
    for (const auto i : algo::range(target.size() & 3))
    {
        *target_u8 = ((table[table_pos++] >> 6) ^ *target_u8) + 0x37;
        target_u8++;
        table_pos %= table.size();
    }
}

void cpz5::decrypt_1c(
    algo::ptr<u8> target,
    const Plugin &plugin,
    const u32 key,
    const std::array<u32, 4> &hash)
{
    std::array<u32, 4> table;
    for (const auto i : algo::range(4))
        table[i] = hash[i] ^ (key + plugin.crypt_1c.addends[i]);

    size_t table_pos = 0;

    auto target_u32 = reinterpret_cast<u32*>(&target[0]);
    u32 seed = plugin.crypt_1c.init_key;
    for (const auto i : algo::range(target.size() >> 2))
    {
        const u32 tmp = (*target_u32 ^ table[table_pos++]) - seed;
        *target_u32++ = algo::rotl<u32>(tmp, 2) + 0x37A19E8B;
        table_pos %= table.size();
        seed -= 0x139FA9B;
    }

    auto target_u8 = reinterpret_cast<u8*>(target_u32);
    for (const auto i : algo::range(target.size() & 3))
    {
        *target_u8 = ((table[table_pos++] >> 4) ^ *target_u8) + 0x05;
        target_u8++;
        table_pos %= table.size();
    }
}

void cpz5::decrypt_2(
    algo::ptr<u8> target,
    const Plugin &plugin,
    const u32 key,
    const u32 seed,
    const u8 permutation_xor)
{
    const auto table
        = get_table_for_crypt_2_and_3(plugin, key, seed);
    for (const auto i : algo::range(target.size()))
        target[i] = table[target[i] ^ permutation_xor];
}

void cpz5::decrypt_3(
    algo::ptr<u8> target,
    const Plugin &plugin,
    const u32 key,
    const u32 seed,
    const std::array<u32, 4> &hash,
    const u32 entry_key)
{
    const auto table
        = get_table_for_crypt_2_and_3(plugin, key, seed);

    u32 yet_another_table[24];
    for (const auto i : algo::range(96))
    {
        reinterpret_cast<u8*>(yet_another_table)[i]
            = table[plugin.secret[i]] ^ (hash[1] >> 2);
    }

    for (const auto i : algo::range(24))
        yet_another_table[i] ^= entry_key;

    size_t yet_another_table_pos = plugin.crypt_3.start_pos;
    u32 yet_another_key = 0x2547A39E;

    u32 *target_u32 = reinterpret_cast<u32*>(&target[0]);
    for (const auto i : algo::range(target.size() >> 2))
    {
        const auto tmp1 = *target_u32;
        const auto tmp2 = yet_another_table[yet_another_table_pos & 0x0F] >> 1;
        const auto tmp3 = yet_another_table[(yet_another_key >> 6) & 0x0F];
        const auto final_tmp = tmp1 ^ tmp2 ^ tmp3;
        *target_u32 = hash[yet_another_key & 3] ^ (final_tmp - entry_key);

        yet_another_table_pos++;
        yet_another_key += *target_u32 + entry_key;
        target_u32++;
    }

    u8 *target_u8 = reinterpret_cast<u8*>(target_u32);
    for (const auto i : algo::range(target.size() & 3))
        target_u8[i] = table[target_u8[i] ^ plugin.crypt_3.tail_xor];
}
