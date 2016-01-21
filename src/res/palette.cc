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
    for (auto i : algo::range(color_count))
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
    for (auto i : algo::range(size()))
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
