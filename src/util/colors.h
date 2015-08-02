#ifndef AU_UTIL_COLORS_H
#define AU_UTIL_COLORS_H
#include "types.h"

namespace au {
namespace util {
namespace color {

    enum Channel
    {
        Red   = 0,
        Green = 1,
        Blue  = 2,
        Alpha = 3
    };

    u32 rgb888(u8 r, u8 g, u8 b);
    u32 rgb565(u16 word);
    u32 rgba5551(u16 word);
    u32 rgba4444(u16 word);
    u32 rgba_gray(u8 byte);

    void set_alpha(u32 &color, u8 value);
    void set_channel(u32 &color, Channel channel, u8 value);
    u8 get_channel(u32 color, Channel channel);

    void split_channels(u32 color, u8 channels[4]);
    void merge_channels(u8 channels[4], u32 &color);

} } }

#endif
