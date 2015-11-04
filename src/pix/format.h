#pragma once

#include "io/io.h"
#include "pix/pixel.h"

namespace au {
namespace pix {

    enum class Format : u8
    {
        Gray8,
        BGR555X,
        BGR565,
        BGR888,
        BGR888X,
        BGRA4444,
        BGRA5551,
        BGRA8888,
        RGB888,
        RGBA8888,
    };

    // TODO: constexpr once cygwin adapts mingw-w64 for its g++
    inline int format_to_bpp(Format fmt)
    {
        switch (fmt)
        {
            case Format::Gray8: return 1;
            case Format::BGR555X: return 2;
            case Format::BGR565: return 2;
            case Format::BGR888: return 3;
            case Format::BGR888X: return 4;
            case Format::BGRA4444: return 2;
            case Format::BGRA5551: return 2;
            case Format::BGRA8888: return 4;
            case Format::RGB888: return 3;
            case Format::RGBA8888: return 4;
            default: return 0;
        }
    }

    template<Format fmt> Pixel read(const u8 *&ptr);

    template<Format fmt> void read_many(
        const u8 *input_ptr, std::vector<Pixel> &output)
    {
        for (auto &c : output)
            c = read<fmt>(input_ptr);
    }

    void read_many(const u8 *input_ptr, std::vector<Pixel> &output, Format fmt);

    // TODO: constexpr once cygwin adapts mingw-w64 for its g++
    template<Format fmt> inline Pixel read(io::IO &io)
    {
        auto str = io.read(format_to_bpp(fmt));
        auto str_ptr = str.get<const u8>();
        return read<fmt>(str_ptr);
    }

} }
