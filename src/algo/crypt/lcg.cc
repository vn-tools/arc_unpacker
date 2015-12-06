#include "algo/crypt/lcg.h"

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
    s32 hi = x / q;
    s32 lo = x % q;
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
