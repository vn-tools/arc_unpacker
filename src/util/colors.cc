#include "util/colors.h"

u32 au::util::color::rgb565(u16 word)
{
     return ((word & 0x1F) << 3)
        | ((word & 0x7E0) << 5)
        | ((word & 0xF800) << 8)
        | 0xFF000000;
}

u32 au::util::color::rgba4444(u16 word)
{
    return ((word & 0xF) << 4)
        | ((word & 0xF0) << 8)
        | ((word & 0xF00) << 12)
        | ((word & 0xF000) << 16);
}

u32 au::util::color::rgba5551(u16 word)
{
    return ((word & 0x1F) << 3)
        | ((word & 0x3E0) << 6)
        | ((word & 0x7C00) << 9)
        | (word & 0x8000 ? 0xFF000000 : 0);
}

u32 au::util::color::rgba_gray(u8 byte)
{
    return byte | (byte << 8) | (byte << 16) | 0xFF000000;
}

void au::util::color::set_channel(u32 &color, u8 channel, u8 value)
{
    auto shift = channel << 3;
    color &= ~(0xFF << shift);
    color |= value << shift;
}

u8 au::util::color::get_channel(u32 color, u8 channel)
{
    auto shift = channel << 3;
    return (color >> shift) & 0xFF;
}

void au::util::color::split_channels(u32 color, u8 channels[4])
{
    for (auto i = 0; i < 4; i++)
        channels[i] = get_channel(color, i);
}

void au::util::color::merge_channels(u8 channels[4], u32 &color)
{
    for (auto i = 0; i < 4; i++)
        set_channel(color, i, channels[i]);
}
