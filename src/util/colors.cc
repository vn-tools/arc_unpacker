#include "util/colors.h"

uint32_t rgb565(uint16_t word)
{
     return ((word & 0x1f) << 3)
        | ((word & 0x7e0) << 5)
        | ((word & 0xf800) << 8)
        | 0xff000000;
}

uint32_t rgba4444(uint16_t word)
{
    return ((word & 0xf) << 4)
        | ((word & 0xf0) << 8)
        | ((word & 0xf00) << 12)
        | ((word & 0xf000) << 16);
}

uint32_t rgba5551(uint16_t word)
{
    return ((word & 0x1f) << 3)
        | ((word & 0x3e0) << 6)
        | ((word & 0x7c00) << 9)
        | (word & 0x8000 ? 0xff000000 : 0);
}

uint32_t rgba_gray(uint8_t byte)
{
    return byte | (byte << 8) | (byte << 16) | 0xff000000;
}

void set_channel(uint32_t &color, uint8_t channel, uint8_t value)
{
    auto shift = channel << 3;
    color &= ~(0xff << shift);
    color |= value << shift;
}

uint8_t get_channel(uint32_t color, uint8_t channel)
{
    auto shift = channel << 3;
    return (color >> shift) & 0xff;
}

void split_channels(uint32_t color, uint8_t channels[4])
{
    for (auto i = 0; i < 4; i ++)
        channels[i] = get_channel(color, i);
}

void merge_channels(uint8_t channels[4], uint32_t &color)
{
    for (auto i = 0; i < 4; i ++)
        set_channel(color, i, channels[i]);
}
