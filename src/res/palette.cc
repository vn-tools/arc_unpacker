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

#include "res/palette.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::res;

struct Palette::Priv final
{
    Priv(const size_t color_count);
    Priv(const size_t color_count, const bstr &input, const PixelFormat fmt);

    std::vector<Pixel> colors;
};

Palette::Priv::Priv(const size_t color_count)
{
    colors.resize(color_count);
    for (const auto i : algo::range(color_count))
    {
        colors[i].r = colors[i].g = colors[i].b = i * 0xFF / color_count;
        colors[i].a = 0xFF;
    }
}

Palette::Priv::Priv(
    const size_t color_count, const bstr &input, const PixelFormat fmt)
{
    colors.resize(color_count);
    if (input.size() < pixel_format_to_bpp(fmt) * color_count)
        throw err::BadDataSizeError();
    read_pixels(input.get<const u8>(), colors, fmt);
}

Palette::Palette(const Palette &other) : p(new Priv(other.p->colors.size()))
{
    for (const auto i : algo::range(size()))
        p->colors[i] = other.p->colors[i];
}

Palette::Palette(const size_t colors) : p(new Priv(colors))
{
}

Palette::Palette(const size_t colors, const bstr &input, const PixelFormat fmt)
    : p(new Priv(colors, input, fmt))
{
}

Palette::Palette(
    const size_t colors,
    io::BaseByteStream &input_stream,
    const PixelFormat fmt) :
        p(new Priv(
            colors, input_stream.read(pixel_format_to_bpp(fmt) * colors), fmt))
{
}

Palette::~Palette()
{
}

size_t Palette::size() const
{
    return p->colors.size();
}

Pixel &Palette::at(const size_t i)
{
    return p->colors.at(i);
}

const Pixel &Palette::at(const size_t i) const
{
    return p->colors.at(i);
}

Pixel &Palette::operator [](const size_t i)
{
    return p->colors[i];
}

const Pixel &Palette::operator [](const size_t i) const
{
    return p->colors[i];
}

Pixel *Palette::begin()
{
    return p->colors.empty() ? nullptr : &p->colors[0];
}

Pixel *Palette::end()
{
    return p->colors.empty() ? nullptr : begin() + p->colors.size();
}

const Pixel *Palette::begin() const
{
    return p->colors.empty() ? nullptr : &p->colors[0];
}

const Pixel *Palette::end() const
{
    return p->colors.empty() ? nullptr : begin() + p->colors.size();
}
