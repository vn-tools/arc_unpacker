#include "pix/palette.h"
#include "err.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::pix;

struct Palette::Priv final
{
    Priv(size_t color_count);
    Priv(size_t color_count, const bstr &input, Format fmt);

    std::vector<Pixel> colors;
};

Palette::Priv::Priv(size_t color_count)
{
    colors.resize(color_count);
    for (auto i : util::range(color_count))
    {
        colors[i].r = colors[i].g = colors[i].b = i * 0xFF / color_count;
        colors[i].a = 0xFF;
    }
}

Palette::Priv::Priv(size_t color_count, const bstr &input, Format fmt)
{
    colors.resize(color_count);
    if (input.size() < format_to_bpp(fmt) * color_count)
        throw err::BadDataSizeError();
    read_many(input.get<const u8>(), colors, fmt);
}

Palette::Palette(const Palette &other) : p(new Priv(other.p->colors.size()))
{
    for (auto i : util::range(size()))
        p->colors[i] = other.p->colors[i];
}

Palette::Palette(size_t colors) : p(new Priv(colors))
{
}

Palette::Palette(size_t colors, const bstr &input, Format fmt)
    : p(new Priv(colors, input, fmt))
{
}

Palette::Palette(size_t colors, io::Stream &input_stream, Format fmt)
    : p(new Priv(colors, input_stream.read(format_to_bpp(fmt) * colors), fmt))
{
}

Palette::~Palette()
{
}

size_t Palette::size() const
{
    return p->colors.size();
}

Pixel &Palette::at(size_t i)
{
    return p->colors.at(i);
}

const Pixel &Palette::at(size_t i) const
{
    return p->colors.at(i);
}

Pixel &Palette::operator [](size_t i)
{
    return p->colors[i];
}

const Pixel &Palette::operator [](size_t i) const
{
    return p->colors[i];
}

Pixel *Palette::begin()
{
    return &p->colors[0];
}

Pixel *Palette::end()
{
    return &p->colors[p->colors.size()];
}

const Pixel *Palette::begin() const
{
    return &p->colors[0];
}

const Pixel *Palette::end() const
{
    return &p->colors[p->colors.size()];
}
