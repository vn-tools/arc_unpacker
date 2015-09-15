#include "pix/palette.h"
#include "err.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::pix;

template<Format fmt> static void read_many(
    const u8 *input_ptr, std::vector<Pixel> &colors)
{
    for (auto i : util::range(colors.size()))
        colors[i] = read<fmt>(input_ptr);
}

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
    auto input_ptr = input.get<const u8>();

    //anyone knows of sane alternative?
    switch (fmt)
    {
        case Format::Gray8:
            read_many<Format::Gray8>(input_ptr, colors);
            break;

        case Format::BGR888:
            read_many<Format::BGR888>(input_ptr, colors);
            break;

        case Format::BGR888X:
            read_many<Format::BGR888X>(input_ptr, colors);
            break;

        case Format::BGRA8888:
            read_many<Format::BGRA8888>(input_ptr, colors);
            break;

        case Format::BGR565:
            read_many<Format::BGR565>(input_ptr, colors);
            break;

        case Format::BGRA5551:
            read_many<Format::BGRA5551>(input_ptr, colors);
            break;

        case Format::BGRA4444:
            read_many<Format::BGRA4444>(input_ptr, colors);
            break;

        case Format::RGB888:
            read_many<Format::RGB888>(input_ptr, colors);
            break;

        case Format::RGBA8888:
            read_many<Format::RGBA8888>(input_ptr, colors);
            break;

        default:
            throw std::logic_error(
                util::format("Unsupported pixel format: %d", fmt));
    }
}

Palette::Palette(size_t colors) : p(new Priv(colors))
{
}

Palette::Palette(size_t colors, const bstr &input, Format fmt)
    : p(new Priv(colors, input, fmt))
{
}

Palette::Palette(size_t colors, io::IO &input_io, Format fmt)
    : p(new Priv(colors, input_io.read(format_to_bpp(fmt) * colors), fmt))
{
}

Palette::~Palette()
{
}

size_t Palette::size() const
{
    return p->colors.size();
}

Pixel &Palette::operator [](size_t i)
{
    return p->colors[i];
}

const Pixel &Palette::operator [](size_t i) const
{
    return p->colors[i];
}
