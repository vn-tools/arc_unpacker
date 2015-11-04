#include "pix/format.h"
#include <cstring>
#include "util/format.h"
#include "util/range.h"

namespace au {
namespace pix {

    template<> Pixel read<Format::Gray8>(const u8 *&ptr)
    {
        Pixel c;
        c.b = c.g = c.r = *ptr++;
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read<Format::BGR555X>(const u8 *&ptr)
    {
        Pixel c;
        u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.b = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000011'11100000) >> 2;
        c.r = (tmp & 0b01111100'00000000) >> 7;
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

    template<> Pixel read<Format::BGRA8888>(const u8 *&ptr)
    {
        Pixel c;
        c.b = *ptr++;
        c.g = *ptr++;
        c.r = *ptr++;
        c.a = *ptr++;
        return c;
    }

    template<> Pixel read<Format::BGRnA4444>(const u8 *&ptr)
    {
        Pixel c;
        u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.b = (tmp & 0b00000000'00001111) << 4;
        c.g = (tmp & 0b00000000'11110000) << 0;
        c.r = (tmp & 0b00001111'00000000) >> 4;
        c.a = ((tmp & 0b11110000'00000000) >> 8) ^ 0xFF;
        return c;
    }

    template<> Pixel read<Format::BGRnA5551>(const u8 *&ptr)
    {
        Pixel c;
        u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.b = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000011'11100000) >> 2;
        c.r = (tmp & 0b01111100'00000000) >> 7;
        c.a = (tmp & 0b10000000'00000000) ? 0 : 0xFF;
        return c;
    }

    template<> Pixel read<Format::BGRnA8888>(const u8 *&ptr)
    {
        Pixel c;
        c.b = *ptr++;
        c.g = *ptr++;
        c.r = *ptr++;
        c.a = *ptr++ ^ 0xFF;
        return c;
    }

    template<> Pixel read<Format::RGB555X>(const u8 *&ptr)
    {
        Pixel c;
        u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.r = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000011'11100000) >> 2;
        c.b = (tmp & 0b01111100'00000000) >> 7;
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read<Format::RGB565>(const u8 *&ptr)
    {
        Pixel c;
        u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.r = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000111'11100000) >> 3;
        c.b = (tmp & 0b11111000'00000000) >> 8;
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

    template<> Pixel read<Format::RGB888X>(const u8 *&ptr)
    {
        Pixel c;
        c.r = *ptr++;
        c.g = *ptr++;
        c.b = *ptr++;
        c.a = 0xFF | *ptr++;
        return c;
    }

    template<> Pixel read<Format::RGBA4444>(const u8 *&ptr)
    {
        Pixel c;
        u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.r = (tmp & 0b00000000'00001111) << 4;
        c.g = (tmp & 0b00000000'11110000) << 0;
        c.b = (tmp & 0b00001111'00000000) >> 4;
        c.a = (tmp & 0b11110000'00000000) >> 8;
        return c;
    }

    template<> Pixel read<Format::RGBA5551>(const u8 *&ptr)
    {
        Pixel c;
        u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.r = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000011'11100000) >> 2;
        c.b = (tmp & 0b01111100'00000000) >> 7;
        c.a = (tmp & 0b10000000'00000000) ? 0xFF : 0;
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

    template<> Pixel read<Format::RGBnA4444>(const u8 *&ptr)
    {
        Pixel c;
        u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.r = (tmp & 0b00000000'00001111) << 4;
        c.g = (tmp & 0b00000000'11110000) << 0;
        c.b = (tmp & 0b00001111'00000000) >> 4;
        c.a = ((tmp & 0b11110000'00000000) >> 8) ^ 0xFF;
        return c;
    }

    template<> Pixel read<Format::RGBnA5551>(const u8 *&ptr)
    {
        Pixel c;
        u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.r = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000011'11100000) >> 2;
        c.b = (tmp & 0b01111100'00000000) >> 7;
        c.a = (tmp & 0b10000000'00000000) ? 0 : 0xFF;
        return c;
    }

    template<> Pixel read<Format::RGBnA8888>(const u8 *&ptr)
    {
        Pixel c;
        c.r = *ptr++;
        c.g = *ptr++;
        c.b = *ptr++;
        c.a = *ptr++ ^ 0xFF;
        return c;
    }

    void read_many(const u8 *input_ptr, std::vector<Pixel> &output, Format fmt)
    {
        // save those precious CPU cycles
        if (fmt == Format::BGRA8888)
        {
            std::memcpy(&output[0], input_ptr, output.size() * 4);
            return;
        }

        // I don't think there is a better alternative to this
        std::function<void(const u8 *, std::vector<Pixel> &)> impl;
        switch (fmt)
        {
            case Format::Gray8:     impl = read_many<Format::Gray8>; break;
            case Format::BGR555X:   impl = read_many<Format::BGR555X>; break;
            case Format::BGR565:    impl = read_many<Format::BGR565>; break;
            case Format::BGR888:    impl = read_many<Format::BGR888>; break;
            case Format::BGR888X:   impl = read_many<Format::BGR888X>; break;
            case Format::BGRA4444:  impl = read_many<Format::BGRA4444>; break;
            case Format::BGRA5551:  impl = read_many<Format::BGRA5551>; break;
            case Format::BGRA8888:  impl = read_many<Format::BGRA8888>; break;
            case Format::BGRnA4444: impl = read_many<Format::BGRnA4444>; break;
            case Format::BGRnA5551: impl = read_many<Format::BGRnA5551>; break;
            case Format::BGRnA8888: impl = read_many<Format::BGRnA8888>; break;
            case Format::RGB555X:   impl = read_many<Format::RGB555X>; break;
            case Format::RGB565:    impl = read_many<Format::RGB565>; break;
            case Format::RGB888:    impl = read_many<Format::RGB888>; break;
            case Format::RGB888X:   impl = read_many<Format::RGB888X>; break;
            case Format::RGBA4444:  impl = read_many<Format::RGBA4444>; break;
            case Format::RGBA5551:  impl = read_many<Format::RGBA5551>; break;
            case Format::RGBA8888:  impl = read_many<Format::RGBA8888>; break;
            case Format::RGBnA4444: impl = read_many<Format::RGBnA4444>; break;
            case Format::RGBnA5551: impl = read_many<Format::RGBnA5551>; break;
            case Format::RGBnA8888: impl = read_many<Format::RGBnA8888>; break;
            default:
                throw std::logic_error(
                    util::format("Unsupported pixel format: %d", fmt));
        }
        impl(input_ptr, output);
    }

} }
