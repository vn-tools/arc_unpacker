#include "res/image.h"
#include <algorithm>
#include "algo/format.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::res;

Image::Image(const Image &other) : Image(other.width(), other.height())
{
    for (auto y : algo::range(_height))
    for (auto x : algo::range(_width))
        at(x, y) = other.at(x, y);
}

Image::Image(const size_t width, const size_t height)
    : pixels(width * height),  _width(width), _height(height)
{
}

Image::Image(
    const size_t width,
    const size_t height,
    const bstr &input,
    const PixelFormat fmt) : Image(width, height)
{
    if (input.size() < pixel_format_to_bpp(fmt) * width * height)
        throw err::BadDataSizeError();
    read_pixels(input.get<const u8>(), pixels, fmt);
}

Image::Image(
    const size_t width,
    const size_t height,
    io::Stream &input_stream,
    const PixelFormat fmt) :
        Image(
            width,
            height,
            input_stream.read(width * height * pixel_format_to_bpp(fmt)),
            fmt)
{
}

Image::Image(
    const size_t width,
    const size_t height,
    const bstr &input,
    const Palette &palette)
        : Image(width, height, input, PixelFormat::Gray8)
{
    apply_palette(palette);
}

Image::Image(
    const size_t width,
    const size_t height,
    io::Stream &input_stream,
    const Palette &palette)
        : Image(width, height, input_stream, PixelFormat::Gray8)
{
    apply_palette(palette);
}

Image::~Image()
{
}

void Image::flip_vertically()
{
    for (auto y : algo::range(_height >> 1))
    for (auto x : algo::range(_width))
    {
        auto t = at(x, _height - 1 - y);
        at(x, _height - 1 - y) = at(x, y);
        at(x, y) = t;
    }
}

void Image::flip_horizontally()
{
    for (auto y : algo::range(_height))
    for (auto x : algo::range(_width >> 1))
    {
        auto t = at(_width - 1 - x, y);
        at(_width - 1 - x, y) = at(x, y);
        at(x, y) = t;
    }
}

void Image::crop(const size_t new_width, const size_t new_height)
{
    std::vector<Pixel> old_pixels(pixels.begin(), pixels.end());
    auto old_width = _width;
    auto old_height = _height;
    _width = new_width;
    _height = new_height;
    pixels.resize(new_width * new_height);
    for (auto y : algo::range(std::min(old_height, new_height)))
    for (auto x : algo::range(std::min(old_width, new_width)))
    {
        pixels[y * new_width + x] = old_pixels[y * old_width + x];
    }
}

void Image::apply_mask(const Image &other)
{
    if (other.width() != _width || other.height() != _height)
        throw std::logic_error("Mask image size is different from image size");
    for (auto y : algo::range(_height))
    for (auto x : algo::range(_width))
        at(x, y).a = other.at(x, y).r;
}

void Image::apply_palette(const Palette &palette)
{
    auto palette_size = palette.size();
    for (auto &c : pixels)
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
    for (const auto y : algo::range(y1, y2, 1))
    for (const auto x : algo::range(x1, x2, 1))
    {
        const auto &target_pixel = other.at(source_x + x, source_y + y);
        if (target_pixel.a)
            at(x, y) = target_pixel;
    }
}

Pixel *Image::begin()
{
    return pixels.empty() ? nullptr : &pixels[0];
}

Pixel *Image::end()
{
    return pixels.empty() ? nullptr : begin() + _width * _height;
}

const Pixel *Image::begin() const
{
    return pixels.empty() ? nullptr : &pixels[0];
}

const Pixel *Image::end() const
{
    return pixels.empty() ? nullptr : begin() + _width * _height;
}
