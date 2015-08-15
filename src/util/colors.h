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
        c.b = (word & 0b00000000'00001111) << 4;
        c.g = (word & 0b00000000'11110000) << 0;
        c.r = (word & 0b00001111'00000000) >> 4;
        c.a = (word & 0b11110000'00000000) >> 8;
        return c;
    }

    inline Color bgr565(io::IO &io)
    {
        auto word = io.read_u16_le();
        Color c;
        c.b = (word & 0b00000000'00011111) << 3;
        c.g = (word & 0b00000111'11100000) >> 3;
        c.r = (word & 0b11111000'00000000) >> 8;
        c.a = 0xFF;
        return c;
    }

    inline Color bgra5551(io::IO &io)
    {
        auto word = io.read_u16_le();
        Color c;
        c.b = (word & 0b00000000'00011111) << 3;
        c.g = (word & 0b00000011'11100000) >> 2;
        c.r = (word & 0b01111100'00000000) >> 7;
        c.a = (word & 0b10000000'00000000) ? 0xFF : 0;
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
