#include "pix/format.h"

namespace au {
namespace pix {

    template<> Pixel read<Format::Gray8>(const u8 *&ptr)
    {
        Pixel c;
        c.b = c.g = c.r = *ptr++;
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read<Format::BGR888>(const u8 *&ptr)
    {
        Pixel c;
        c.b = *ptr++;
        c.g = *ptr++;
        c.r = *ptr++;
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read<Format::BGR888X>(const u8 *&ptr)
    {
        Pixel c;
        c.b = *ptr++;
        c.g = *ptr++;
        c.r = *ptr++;
        c.a = 0xFF | *ptr++;
        return c;
    }

    template<> Pixel read<Format::BGRA8888>(const u8 *&ptr)
    {
        Pixel c;
        c.b = *ptr++;
        c.g = *ptr++;
        c.r = *ptr++;
        c.a = *ptr++;
        return c;
    }

    template<> Pixel read<Format::BGRA4444>(const u8 *&ptr)
    {
        Pixel c;
        u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.b = (tmp & 0b00000000'00001111) << 4;
        c.g = (tmp & 0b00000000'11110000) << 0;
        c.r = (tmp & 0b00001111'00000000) >> 4;
        c.a = (tmp & 0b11110000'00000000) >> 8;
        return c;
    }

    template<> Pixel read<Format::BGRA5551>(const u8 *&ptr)
    {
        Pixel c;
        u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.b = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000011'11100000) >> 2;
        c.r = (tmp & 0b01111100'00000000) >> 7;
        c.a = (tmp & 0b10000000'00000000) ? 0xFF : 0;
        return c;
    }

    template<> Pixel read<Format::BGR555X>(const u8 *&ptr)
    {
        Pixel c = read<Format::BGRA5551>(ptr);
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read<Format::BGR565>(const u8 *&ptr)
    {
        Pixel c;
        u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.b = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000111'11100000) >> 3;
        c.r = (tmp & 0b11111000'00000000) >> 8;
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read<Format::RGB888>(const u8 *&ptr)
    {
        Pixel c;
        c.r = *ptr++;
        c.g = *ptr++;
        c.b = *ptr++;
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read<Format::RGBA8888>(const u8 *&ptr)
    {
        Pixel c;
        c.r = *ptr++;
        c.g = *ptr++;
        c.b = *ptr++;
        c.a = *ptr++;
        return c;
    }

} }
