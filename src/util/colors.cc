#include "util/colors.h"
#include "util/range.h"

using namespace au;

u32 util::color::rgb888(u8 r, u8 g, u8 b)
{
    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

u32 util::color::rgb565(u16 word)
{
     return ((word & 0x1F) << 3)
        | ((word & 0x7E0) << 5)
        | ((word & 0xF800) << 8)
        | 0xFF000000;
}

u32 util::color::rgba4444(u16 word)
{
    return ((word & 0xF) << 4)
        | ((word & 0xF0) << 8)
        | ((word & 0xF00) << 12)
        | ((word & 0xF000) << 16);
}

u32 util::color::rgba5551(u16 word)
{
    return ((word & 0x1F) << 3)
        | ((word & 0x3E0) << 6)
        | ((word & 0x7C00) << 9)
        | (word & 0x8000 ? 0xFF000000 : 0);
}

u32 util::color::rgba_gray(u8 byte)
{
    return util::color::rgb888(byte, byte, byte);
}

void util::color::set_alpha(u32 &color, u8 value)
{
    util::color::set_channel(color, Channel::Alpha, value);
}

void util::color::set_channel(u32 &color, Channel channel, u8 value)
{
    auto shift = channel << 3;
    color &= ~(0xFF << shift);
    color |= value << shift;
}

u8 util::color::get_channel(u32 color, Channel channel)
{
    auto shift = channel << 3;
    return (color >> shift) & 0xFF;
}

void util::color::split_channels(u32 color, u8 channels[4])
{
    for (auto i : util::range(4))
        channels[i] = get_channel(color, static_cast<util::color::Channel>(i));
}

void util::color::merge_channels(u8 channels[4], u32 &color)
{
    for (auto i : util::range(4))
        set_channel(color, static_cast<util::color::Channel>(i), channels[i]);
}
