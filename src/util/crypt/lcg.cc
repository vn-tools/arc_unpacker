#include "util/crypt/lcg.h"
#include "util/require.h"

using namespace au;
using namespace au::util::crypt;

struct Lcg::Priv
{
    u32 seed;
    u32 multiplier;
    u32 increment;
    u8 shift;
    u32 mask;
};

Lcg::Lcg(LcgKind kind, u32 seed) : p(new Priv)
{
    p->seed = seed;
    switch (kind)
    {
        case LcgKind::MicrosoftVisualC:
            p->multiplier = 0x343FD;
            p->increment = 0x269EC3;
            p->shift = 16;
            p->mask = 0x7FFF;
            break;

        default:
            util::fail("Unknown LCG kind");
    }
}

Lcg::~Lcg()
{
}

u32 Lcg::next()
{
    p->seed = p->seed * p->multiplier + p->increment;
    return (p->seed >> p->shift) & p->mask;
}
