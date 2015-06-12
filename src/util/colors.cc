#include "util/colors.h"

u32 rgb565(u16 word)
{
     return ((word & 0x1f) << 3)
        | ((word & 0x7e0) << 5)
        | ((word & 0xf800) << 8)
        | 0xff000000;
}

u32 rgba4444(u16 word)
{
    return ((word & 0xf) << 4)
        | ((word & 0xf0) << 8)
        | ((word & 0xf00) << 12)
        | ((word & 0xf000) << 16);
}

u32 rgba5551(u16 word)
{
    return ((word & 0x1f) << 3)
        | ((word & 0x3e0) << 6)
        | ((word & 0x7c00) << 9)
        | (word & 0x8000 ? 0xff000000 : 0);
}

u32 rgba_gray(u8 byte)
{
    return byte | (byte << 8) | (byte << 16) | 0xff000000;
}

void set_channel(u32 &color, u8 channel, u8 value)
{
    auto shift = channel << 3;
    color &= ~(0xff << shift);
    color |= value << shift;
}

u8 get_channel(u32 color, u8 channel)
{
    auto shift = channel << 3;
    return (color >> shift) & 0xff;
}

void split_channels(u32 color, u8 channels[4])
{
    for (auto i = 0; i < 4; i ++)
        channels[i] = get_channel(color, i);
}

void merge_channels(u8 channels[4], u32 &color)
{
    for (auto i = 0; i < 4; i ++)
        set_channel(color, i, channels[i]);
}
