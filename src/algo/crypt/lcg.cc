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

#include "algo/crypt/lcg.h"
#include <functional>

using namespace au;
using namespace au::algo::crypt;

struct Lcg::Priv final
{
    u32 seed;
    std::function<u32(u32&)> next;
};

static inline u32 minstd(u32 &seed, u32 a, u32 q, u32 r, u32 m)
{
    s32 x = seed;
    const s32 hi = x / q;
    const s32 lo = x % q;
    x = a * lo - r * hi;
    if (x < 0)
        x += m;
    seed = x;
    return x * 4.656612875245797e-10 * 256;
}

Lcg::Lcg(LcgKind kind, u32 seed) : p(new Priv)
{
    p->seed = seed;
    switch (kind)
    {
        case LcgKind::MicrosoftVisualC:
            p->next = [](u32 &seed)
            {
                seed = seed * 0x343FD + 0x269EC3;
                return (seed >> 16) & 0x7FFF;
            };
            break;

        case LcgKind::ParkMiller:
            p->next = [](u32 &seed)
            {
                return minstd(seed, 16807, 127773, 2836, 2147483647);
            };
            break;

        case LcgKind::ParkMillerRevised:
            p->next = [](u32 &seed)
            {
                return minstd(seed, 48271, 44488, 3399, 2147483647);
            };
            break;

        default:
            throw std::logic_error("Unknown LCG kind");
    }
}

Lcg::~Lcg()
{
}

u32 Lcg::next()
{
    return p->next(p->seed);
}
