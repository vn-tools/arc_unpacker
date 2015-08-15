#ifndef AU_UTIL_COLORS_H
#define AU_UTIL_COLORS_H
#include "types.h"
#include "io/io.h"
#include "util/image.h"

namespace au {
namespace util {
namespace color {

    inline Color gray8(io::IO &io)
    {
        Color c;
        c.r = c.g = c.b = io.read_u8();
        c.a = 0xFF;
        return c;
    }

    inline Color bgra4444(io::IO &io)
    {
        auto word = io.read_u16_le();
        Color c;
        c.b = (word & 0x000F) << 4;
        c.g = (word & 0x00F0) << 0;
        c.r = (word & 0x0F00) >> 4;
        c.a = (word & 0xF000) >> 8;
        return c;
    }

    inline Color bgr565(io::IO &io)
    {
        auto word = io.read_u16_le();
        Color c;
        c.b = (word & 0x001F) << 3;
        c.g = (word & 0x07E0) >> 3;
        c.r = (word & 0xF800) >> 8;
        c.a = 0xFF;
        return c;
    }

    inline Color bgra5551(io::IO &io)
    {
        auto word = io.read_u16_le();
        Color c;
        c.b = (word & 0x001F) << 3;
        c.g = (word & 0x03E0) >> 2;
        c.r = (word & 0x7C00) >> 7;
        c.a = (word & 0x8000) ? 0xFF : 0;
        return c;
    }

    inline Color bgra8888(io::IO &io)
    {
        Color c;
        c.b = io.read_u8();
        c.g = io.read_u8();
        c.r = io.read_u8();
        c.a = io.read_u8();
        return c;
    }

    inline Color bgr888(io::IO &io)
    {
        Color c;
        c.b = io.read_u8();
        c.g = io.read_u8();
        c.r = io.read_u8();
        c.a = 0xFF;
        return c;
    }

    inline Color rgb888(io::IO &io)
    {
        Color c;
        c.r = io.read_u8();
        c.g = io.read_u8();
        c.b = io.read_u8();
        c.a = 0xFF;
        return c;
    }

} } }

#endif
