#include "pix/image.h"
#include <algorithm>
#include "err.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::pix;

struct Image::Priv final
{
    void load(const bstr &input, const PixelFormat fmt);

    std::vector<Pixel> pixels;
    size_t width;
    size_t height;
};

void Image::Priv::load(const bstr &input, const PixelFormat fmt)
{
    if (input.size() < pixel_format_to_bpp(fmt) * width * height)
        throw err::BadDataSizeError();
    read_pixels(input.get<const u8>(), pixels, fmt);
}

Image::Image(const Image &other) : Image(other.width(), other.height())
{
    for (auto y : util::range(p->height))
    for (auto x : util::range(p->width))
        at(x, y) = other.at(x, y);
}

Image::Image(const size_t width, const size_t height) : p(new Priv)
{
    p->width = width;
    p->height = height;
    p->pixels.resize(width * height);
}

Image::Image(
    const size_t width,
    const size_t height,
    const bstr &input,
    const PixelFormat fmt) : Image(width, height)
{
    p->load(input, fmt);
}

Image::Image(
    const size_t width,
    const size_t height,
    io::Stream &input,
    const PixelFormat fmt) : Image(width, height)
{
    auto bpp = pixel_format_to_bpp(fmt);
    p->load(input.read(width * height * bpp), fmt);
}

Image::Image(
    const size_t width,
    const size_t height,
    const bstr &input,
    const Palette &palette) : Image(width, height)
{
    p->load(input, pix::PixelFormat::Gray8);
    apply_palette(palette);
}

Image::Image(
    const size_t width,
    const size_t height,
    io::Stream &stream,
    const Palette &palette) : Image(width, height)
{
    p->load(stream.read(width * height), pix::PixelFormat::Gray8);
    apply_palette(palette);
}

Image::~Image()
{
}

size_t Image::width() const
{
    return p->width;
}

size_t Image::height() const
{
    return p->height;
}

Pixel &Image::at(const size_t x, const size_t y)
{
    return p->pixels[x + y * p->width];
}

const Pixel &Image::at(const size_t x, const size_t y) const
{
    return p->pixels[x + y * p->width];
}

void Image::flip_vertically()
{
    for (auto y : util::range(p->height >> 1))
    for (auto x : util::range(p->width))
    {
        auto t = at(x, p->height - 1 - y);
        at(x, p->height - 1 - y) = at(x, y);
        at(x, y) = t;
    }
}

void Image::flip_horizontally()
{
    for (auto y : util::range(p->height))
    for (auto x : util::range(p->width >> 1))
    {
        auto t = at(p->width - 1 - x, y);
        at(p->width - 1 - x, y) = at(x, y);
        at(x, y) = t;
    }
}

void Image::crop(const size_t new_width, const size_t new_height)
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

void Image::apply_mask(const Image &other)
{
    if (other.width() != p->width || other.height() != p->height)
        throw std::logic_error("Mask image size is different from image size");
    for (auto y : util::range(p->height))
    for (auto x : util::range(p->width))
        at(x, y).a = other.at(x, y).r;
}

void Image::apply_palette(const Palette &palette)
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

void Image::paste(const Image &other, const int target_x, const int target_y)
{
    const size_t x1 = std::max<size_t>(0, target_x);
    const size_t x2 = std::min<size_t>(width(), target_x + other.width());
    const size_t y1 = std::max<size_t>(0, target_y);
    const size_t y2 = std::min<size_t>(height(), target_y + other.height());
    const size_t source_x = std::max<size_t>(0, -target_x);
    const size_t source_y = std::max<size_t>(0, -target_y);
    for (const auto y : util::range(y1, y2, 1))
    for (const auto x : util::range(x1, x2, 1))
        at(x, y) = other.at(source_x + x, source_y + y);
}

Pixel *Image::begin()
{
    return &p->pixels[0];
}

Pixel *Image::end()
{
    return &p->pixels[p->width * p->height];
}

const Pixel *Image::begin() const
{
    return &p->pixels[0];
}

const Pixel *Image::end() const
{
    return &p->pixels[p->width * p->height];
}
