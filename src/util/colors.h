#ifndef AU_UTIL_COLORS_H
#define AU_UTIL_COLORS_H
#include "types.h"

namespace au {
namespace util {
namespace color {

    u32 rgb565(u16 word);
    u32 rgba5551(u16 word);
    u32 rgba4444(u16 word);
    u32 rgba_gray(u8 byte);

    void set_channel(u32 &color, u8 channel, u8 value);
    u8 get_channel(u32 color, u8 channel);

    void split_channels(u32 color, u8 channels[4]);
    void merge_channels(u8 channels[4], u32 &color);

} } }

#endif
