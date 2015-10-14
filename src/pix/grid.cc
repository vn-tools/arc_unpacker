#include "pix/grid.h"
#include <algorithm>
#include "err.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::pix;

template<Format fmt> static void read_many(
    size_t size, Pixel *pixels_ptr, const u8 *input_ptr)
{
    for (auto i : util::range(size))
        *pixels_ptr++ = read<fmt>(input_ptr);
}

struct Grid::Priv final
{
    void load(const bstr &input, Format fmt);

    std::vector<Pixel> pixels;
    size_t width;
    size_t height;
};

void Grid::Priv::load(const bstr &input, Format fmt)
{
    int bpp = format_to_bpp(fmt);
    if (input.size() < width * height * bpp)
        throw err::BadDataSizeError();
    auto size = width * height;
    auto *input_ptr = input.get<const u8>();
    auto *pixels_ptr = &pixels[0];

    // anyone knows of sane alternative?
    switch (fmt)
    {
        case Format::Gray8:
            read_many<Format::Gray8>(size, pixels_ptr, input_ptr);
            break;

        case Format::BGR888:
            read_many<Format::BGR888>(size, pixels_ptr, input_ptr);
            break;

        case Format::BGR888X:
            read_many<Format::BGR888X>(size, pixels_ptr, input_ptr);
            break;

        case Format::BGRA8888:
            read_many<Format::BGRA8888>(size, pixels_ptr, input_ptr);
            break;

        case Format::BGR565:
            read_many<Format::BGR565>(size, pixels_ptr, input_ptr);
            break;

        case Format::BGR555X:
            read_many<Format::BGR555X>(size, pixels_ptr, input_ptr);
            break;

        case Format::BGRA5551:
            read_many<Format::BGRA5551>(size, pixels_ptr, input_ptr);
            break;

        case Format::BGRA4444:
            read_many<Format::BGRA4444>(size, pixels_ptr, input_ptr);
            break;

        case Format::RGB888:
            read_many<Format::RGB888>(size, pixels_ptr, input_ptr);
            break;

        case Format::RGBA8888:
            read_many<Format::RGBA8888>(size, pixels_ptr, input_ptr);
            break;

        default:
            throw std::logic_error(
                util::format("Unsupported pixel format: %d", fmt));
    }
}

Grid::Grid(const Grid &other) : Grid(other.width(), other.height())
{
    for (auto y : util::range(p->height))
    for (auto x : util::range(p->width))
        at(x, y) = other.at(x, y);
}

Grid::Grid(size_t width, size_t height) : p(new Priv)
{
    p->width = width;
    p->height = height;
    p->pixels.resize(width * height);
}

Grid::Grid(size_t width, size_t height, const bstr &input, Format fmt)
    : Grid(width, height)
{
    p->load(input, fmt);
}

Grid::Grid(size_t width, size_t height, io::IO &input_io, Format fmt)
    : Grid(width, height)
{
    auto bpp = format_to_bpp(fmt);
    p->load(input_io.read(width * height * bpp), fmt);
}

Grid::Grid(
    size_t width, size_t height, const bstr &input, const Palette &palette)
    : Grid(width, height)
{
    p->load(input, pix::Format::Gray8);
    apply_palette(palette);
}

Grid::Grid(
    size_t width, size_t height, io::IO &input_io, const Palette &palette)
    : Grid(width, height)
{
    p->load(input_io.read(width * height), pix::Format::Gray8);
    apply_palette(palette);
}

Grid::~Grid()
{
}

size_t Grid::width() const
{
    return p->width;
}

size_t Grid::height() const
{
    return p->height;
}

Pixel &Grid::at(size_t x, size_t y)
{
    return p->pixels[x + y * p->width];
}

const Pixel &Grid::at(size_t x, size_t y) const
{
    return p->pixels[x + y * p->width];
}

void Grid::flip_vertically()
{
    for (auto y : util::range(p->height >> 1))
    for (auto x : util::range(p->width))
    {
        auto t = at(x, p->height - 1 - y);
        at(x, p->height - 1 - y) = at(x, y);
        at(x, y) = t;
    }
}

void Grid::flip_horizontally()
{
    for (auto y : util::range(p->height))
    for (auto x : util::range(p->width >> 1))
    {
        auto t = at(p->width - 1 - x, y);
        at(p->width - 1 - x, y) = at(x, y);
        at(x, y) = t;
    }
}

void Grid::crop(size_t new_width, size_t new_height)
{
    std::vector<Pixel> old_pixels(p->pixels.begin(), p->pixels.end());
    auto old_width = p->width;
    auto old_height = p->height;
    p->width = new_width;
    p->height = new_height;
    p->pixels.resize(new_width * new_height);
    for (auto y : util::range(std::min(old_height, new_height)))
    for (auto x : util::range(std::min(old_width, new_width)))
    {
        p->pixels[y * new_width + x] = old_pixels[y * old_width + x];
    }
}

void Grid::apply_alpha_from_mask(const Grid &other)
{
    if (other.width() != p->width || other.height() != p->height)
        throw std::logic_error("Mask image size is different from image size");
    for (auto y : util::range(p->height))
    for (auto x : util::range(p->width))
        at(x, y).a = other.at(x, y).r;
}

void Grid::apply_palette(const Palette &palette)
{
    auto palette_size = palette.size();
    for (auto &c : p->pixels)
    {
        if (c.r < palette_size)
            c = palette[c.r];
        else
            c.a = 0x00;
    }
}

Pixel *Grid::begin()
{
    return &p->pixels[0];
}

Pixel *Grid::end()
{
    return &p->pixels[p->width * p->height];
}

const Pixel *Grid::begin() const
{
    return &p->pixels[0];
}

const Pixel *Grid::end() const
{
    return &p->pixels[p->width * p->height];
}
