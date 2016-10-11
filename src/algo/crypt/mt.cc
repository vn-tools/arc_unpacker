// Copyright (C) 1997 by Makoto Matsumoto and Takuji Nishimura
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

#include "algo/crypt/mt.h"
#include <array>
#include "algo/range.h"

using namespace au;
using namespace au::algo::crypt;

static const auto n = 624;
static const auto m = 397;
static const auto matrix_a = 0x9908B0DF;
static const auto upper_mask = 0x80000000;
static const auto lower_mask = 0x7FFFFFFF;

static const auto tempering_mask_b = 0x9D2C5680;
static const auto tempering_mask_c = 0xEFC60000;

static u32 tempering_shift_u(const u32 y) { return y >> 11; }
static u32 tempering_shift_s(const u32 y) { return y << 7; }
static u32 tempering_shift_t(const u32 y) { return y << 15; }
static u32 tempering_shift_l(const u32 y) { return y >> 18; }

struct MersenneTwister::Priv final
{
    std::function<void(Priv*, const u32)> seed_func;

    u32 default_seed;
    std::array<u32, n> mt;
    size_t mti;
};

std::unique_ptr<MersenneTwister> MersenneTwister::Knuth(const u32 seed)
{
    auto ret = std::unique_ptr<MersenneTwister>(new MersenneTwister(
        [](Priv *p, u32 seed)
        {
            p->mt[0] = seed & 0xFFFFFFFF;
            for (p->mti = 1; p->mti < n; p->mti++)
                p->mt[p->mti] = (69069 * p->mt[p->mti - 1]) & 0xFFFFFFFF;
        },
        4357));
    ret->p->seed_func(ret->p.get(), seed);
    return ret;
}

std::unique_ptr<MersenneTwister> MersenneTwister::Classic(const u32 seed)
{
    auto ret = std::unique_ptr<MersenneTwister>(new MersenneTwister(
        [](Priv *p, u32 seed)
        {
            for (const auto i : algo::range(n))
            {
                p->mt[i] = seed & 0xFFFF0000;
                seed = 69069 * seed + 1;
                p->mt[i] |= (seed & 0xFFFF0000) >> 16;
                seed = 69069 * seed + 1;
            }
            p->mti = n;
        },
        4357));
    ret->p->seed_func(ret->p.get(), seed);
    return ret;
}

std::unique_ptr<MersenneTwister> MersenneTwister::Improved(const u32 seed)
{
    auto ret = std::unique_ptr<MersenneTwister>(new MersenneTwister(
        [](Priv *p, u32 seed)
        {
            p->mt[0] = seed & 0xFFFFFFFFul;
            for (p->mti = 1; p->mti < n; p->mti++)
            {
                p->mt[p->mti] = p->mt[p->mti - 1];
                p->mt[p->mti] ^= (p->mt[p->mti - 1] >> 30);
                p->mt[p->mti] = (1812433253ul * p->mt[p->mti] + p->mti);
                p->mt[p->mti] &= 0xFFFFFFFFul;
            }
        },
        5489ul));
    ret->p->seed_func(ret->p.get(), seed);
    return ret;
}

MersenneTwister::MersenneTwister(
    const std::function<void(Priv*, const u32)> seed_func,
    const u32 default_seed)
    : p(new Priv())
{
    p->seed_func = seed_func;
    p->default_seed = default_seed;
}

MersenneTwister::~MersenneTwister()
{
}

u32 MersenneTwister::next_u32()
{
    u32 y;
    static const u32 mag01[2] = {0x0, matrix_a};

    if (p->mti >= n)
    {
        int kk;

        if (p->mti == n + 1)
            p->seed_func(p.get(), p->default_seed);

        for (kk = 0; kk < n - m; kk++)
        {
            y = (p->mt[kk] & upper_mask) | (p->mt[kk + 1] & lower_mask);
            p->mt[kk] = p->mt[kk + m] ^ (y >> 1) ^ mag01[y & 0x1];
        }

        for (; kk < n - 1; kk++)
        {
            y = (p->mt[kk] & upper_mask) | (p->mt[kk + 1] & lower_mask);
            p->mt[kk] = p->mt[kk + (m - n)] ^ (y >> 1) ^ mag01[y & 0x1];
        }

        y = (p->mt[n - 1] & upper_mask) | (p->mt[0] & lower_mask);
        p->mt[n - 1] = p->mt[m - 1] ^ (y >> 1) ^ mag01[y & 0x1];

        p->mti = 0;
    }

    y = p->mt[p->mti++];
    y ^= tempering_shift_u(y);
    y ^= tempering_shift_s(y) & tempering_mask_b;
    y ^= tempering_shift_t(y) & tempering_mask_c;
    y ^= tempering_shift_l(y);
    return y;
}
