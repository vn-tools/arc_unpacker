// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "res/pixel_format.h"
#include <cstring>
#include "algo/format.h"
#include "algo/range.h"

namespace au {
namespace res {

    template<> Pixel read_pixel<PixelFormat::Gray8>(const u8 *&ptr)
    {
        Pixel c;
        c.b = c.g = c.r = *ptr++;
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::BGR555X>(const u8 *&ptr)
    {
        Pixel c;
        const u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.b = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000011'11100000) >> 2;
        c.r = (tmp & 0b01111100'00000000) >> 7;
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::BGR565>(const u8 *&ptr)
    {
        Pixel c;
        const u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.b = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000111'11100000) >> 3;
        c.r = (tmp & 0b11111000'00000000) >> 8;
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::BGR888>(const u8 *&ptr)
    {
        Pixel c;
        c.b = *ptr++;
        c.g = *ptr++;
        c.r = *ptr++;
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::BGR888X>(const u8 *&ptr)
    {
        Pixel c;
        c.b = *ptr++;
        c.g = *ptr++;
        c.r = *ptr++;
        c.a = 0xFF | *ptr++;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::BGRA4444>(const u8 *&ptr)
    {
        Pixel c;
        const u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.b = (tmp & 0b00000000'00001111) << 4;
        c.g = (tmp & 0b00000000'11110000) << 0;
        c.r = (tmp & 0b00001111'00000000) >> 4;
        c.a = (tmp & 0b11110000'00000000) >> 8;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::BGRA5551>(const u8 *&ptr)
    {
        Pixel c;
        const u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.b = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000011'11100000) >> 2;
        c.r = (tmp & 0b01111100'00000000) >> 7;
        c.a = (tmp & 0b10000000'00000000) ? 0xFF : 0;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::BGRA8888>(const u8 *&ptr)
    {
        Pixel c;
        c.b = *ptr++;
        c.g = *ptr++;
        c.r = *ptr++;
        c.a = *ptr++;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::BGRnA4444>(const u8 *&ptr)
    {
        Pixel c;
        const u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.b = (tmp & 0b00000000'00001111) << 4;
        c.g = (tmp & 0b00000000'11110000) << 0;
        c.r = (tmp & 0b00001111'00000000) >> 4;
        c.a = ((tmp & 0b11110000'00000000) >> 8) ^ 0xFF;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::BGRnA5551>(const u8 *&ptr)
    {
        Pixel c;
        const u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.b = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000011'11100000) >> 2;
        c.r = (tmp & 0b01111100'00000000) >> 7;
        c.a = (tmp & 0b10000000'00000000) ? 0 : 0xFF;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::BGRnA8888>(const u8 *&ptr)
    {
        Pixel c;
        c.b = *ptr++;
        c.g = *ptr++;
        c.r = *ptr++;
        c.a = *ptr++ ^ 0xFF;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::RGB555X>(const u8 *&ptr)
    {
        Pixel c;
        const u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.r = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000011'11100000) >> 2;
        c.b = (tmp & 0b01111100'00000000) >> 7;
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::RGB565>(const u8 *&ptr)
    {
        Pixel c;
        const u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.r = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000111'11100000) >> 3;
        c.b = (tmp & 0b11111000'00000000) >> 8;
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::RGB888>(const u8 *&ptr)
    {
        Pixel c;
        c.r = *ptr++;
        c.g = *ptr++;
        c.b = *ptr++;
        c.a = 0xFF;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::RGB888X>(const u8 *&ptr)
    {
        Pixel c;
        c.r = *ptr++;
        c.g = *ptr++;
        c.b = *ptr++;
        c.a = 0xFF | *ptr++;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::RGBA4444>(const u8 *&ptr)
    {
        Pixel c;
        const u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.r = (tmp & 0b00000000'00001111) << 4;
        c.g = (tmp & 0b00000000'11110000) << 0;
        c.b = (tmp & 0b00001111'00000000) >> 4;
        c.a = (tmp & 0b11110000'00000000) >> 8;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::RGBA5551>(const u8 *&ptr)
    {
        Pixel c;
        const u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.r = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000011'11100000) >> 2;
        c.b = (tmp & 0b01111100'00000000) >> 7;
        c.a = (tmp & 0b10000000'00000000) ? 0xFF : 0;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::RGBA8888>(const u8 *&ptr)
    {
        Pixel c;
        c.r = *ptr++;
        c.g = *ptr++;
        c.b = *ptr++;
        c.a = *ptr++;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::RGBnA4444>(const u8 *&ptr)
    {
        Pixel c;
        const u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.r = (tmp & 0b00000000'00001111) << 4;
        c.g = (tmp & 0b00000000'11110000) << 0;
        c.b = (tmp & 0b00001111'00000000) >> 4;
        c.a = ((tmp & 0b11110000'00000000) >> 8) ^ 0xFF;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::RGBnA5551>(const u8 *&ptr)
    {
        Pixel c;
        const u16 tmp = reinterpret_cast<const u16&>(*ptr);
        ptr += 2;
        c.r = (tmp & 0b00000000'00011111) << 3;
        c.g = (tmp & 0b00000011'11100000) >> 2;
        c.b = (tmp & 0b01111100'00000000) >> 7;
        c.a = (tmp & 0b10000000'00000000) ? 0 : 0xFF;
        return c;
    }

    template<> Pixel read_pixel<PixelFormat::RGBnA8888>(const u8 *&ptr)
    {
        Pixel c;
        c.r = *ptr++;
        c.g = *ptr++;
        c.b = *ptr++;
        c.a = *ptr++ ^ 0xFF;
        return c;
    }

    void read_pixels(
        const u8 *input_ptr, std::vector<Pixel> &output, const PixelFormat fmt)
    {
        // save those precious CPU cycles
        if (fmt == PixelFormat::BGRA8888)
        {
            std::memcpy(output.data(), input_ptr, output.size() * 4);
            return;
        }

        // I don't think there is a better alternative to this
        using PF = PixelFormat;
        std::function<void(const u8 *, std::vector<Pixel> &)> impl;
        switch (fmt)
        {
            case PF::Gray8:     impl = read_pixels<PF::Gray8>; break;
            case PF::BGR555X:   impl = read_pixels<PF::BGR555X>; break;
            case PF::BGR565:    impl = read_pixels<PF::BGR565>; break;
            case PF::BGR888:    impl = read_pixels<PF::BGR888>; break;
            case PF::BGR888X:   impl = read_pixels<PF::BGR888X>; break;
            case PF::BGRA4444:  impl = read_pixels<PF::BGRA4444>; break;
            case PF::BGRA5551:  impl = read_pixels<PF::BGRA5551>; break;
            case PF::BGRA8888:  impl = read_pixels<PF::BGRA8888>; break;
            case PF::BGRnA4444: impl = read_pixels<PF::BGRnA4444>; break;
            case PF::BGRnA5551: impl = read_pixels<PF::BGRnA5551>; break;
            case PF::BGRnA8888: impl = read_pixels<PF::BGRnA8888>; break;
            case PF::RGB555X:   impl = read_pixels<PF::RGB555X>; break;
            case PF::RGB565:    impl = read_pixels<PF::RGB565>; break;
            case PF::RGB888:    impl = read_pixels<PF::RGB888>; break;
            case PF::RGB888X:   impl = read_pixels<PF::RGB888X>; break;
            case PF::RGBA4444:  impl = read_pixels<PF::RGBA4444>; break;
            case PF::RGBA5551:  impl = read_pixels<PF::RGBA5551>; break;
            case PF::RGBA8888:  impl = read_pixels<PF::RGBA8888>; break;
            case PF::RGBnA4444: impl = read_pixels<PF::RGBnA4444>; break;
            case PF::RGBnA5551: impl = read_pixels<PF::RGBnA5551>; break;
            case PF::RGBnA8888: impl = read_pixels<PF::RGBnA8888>; break;
            default:
                throw std::logic_error(
                    algo::format("Unsupported pixel format: %d", fmt));
        }
        impl(input_ptr, output);
    }

} }
