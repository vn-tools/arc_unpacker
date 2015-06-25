#include <cassert>
#include <png.h>
#include "util/image.h"

struct Image::Priv
{
    size_t width;
    size_t height;
    std::string data;
    size_t data_size;
    PixelFormat fmt;
};

namespace
{
    void png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
    {
        IO *io = reinterpret_cast<IO*>(png_get_io_ptr(png_ptr));
        io->write(data, length);
    }

    void my_png_read_data(
        png_structp png_ptr, png_bytep data, png_size_t length)
    {
        IO *io = reinterpret_cast<IO*>(png_get_io_ptr(png_ptr));
        io->read(data, length);
    }

    void png_flush(png_structp)
    {
    }
}

Image::Image() : p(new Priv)
{
}

Image::~Image()
{
}

std::unique_ptr<Image> Image::from_pixels(
    size_t width, size_t height, const std::string &data, PixelFormat fmt)
{
    std::unique_ptr<Image> image(new Image);
    if (width == 0 || height == 0)
        throw std::runtime_error("Image dimension cannot be 0");
    image->p->width = width;
    image->p->height = height;
    image->p->data = data;
    image->p->fmt = fmt;
    return image;
}

std::unique_ptr<Image> Image::from_boxed(IO &io)
{
    io.seek(0);

    png_structp png_ptr = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    assert(png_ptr != nullptr);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert(info_ptr != nullptr);

    png_set_read_fn(png_ptr, &io, &my_png_read_data);
    png_read_png(
        png_ptr,
        info_ptr,
        PNG_TRANSFORM_GRAY_TO_RGB
            | PNG_TRANSFORM_STRIP_16
            | PNG_TRANSFORM_PACKING
            | PNG_TRANSFORM_EXPAND,
        nullptr);

    int color_type;
    int bits_per_channel;
    png_uint_32 png_width, png_height;
    png_get_IHDR(
        png_ptr,
        info_ptr,
        &png_width,
        &png_height,
        &bits_per_channel,
        &color_type,
        nullptr,
        nullptr,
        nullptr);
    assert(bits_per_channel == 8);

    std::unique_ptr<Image> image(new Image);
    image->p->width = png_width;
    image->p->height = png_height;

    int bpp = 0;
    switch (color_type)
    {
        case PNG_COLOR_TYPE_RGB:
            bpp = 3;
            image->p->fmt = PixelFormat::RGB;
            break;
        case PNG_COLOR_TYPE_RGBA:
            bpp = 4;
            image->p->fmt = PixelFormat::RGBA;
            break;
        case PNG_COLOR_TYPE_GRAY:
            bpp = 1;
            image->p->fmt = PixelFormat::Grayscale;
            break;
        default:
            throw std::runtime_error("Bad pixel format");
    }

    image->p->data_size = png_width * png_height * bpp;
    image->p->data = std::string(image->p->data_size, '\0');

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
    size_t scanline_size = png_width * bpp, y;
    for (y = 0; y < png_height; y++)
    {
        image->p->data.replace(
            y * scanline_size,
            scanline_size,
            reinterpret_cast<char*>(row_pointers[y]),
            scanline_size);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return image;
}

size_t Image::width() const
{
    return p->width;
}

size_t Image::height() const
{
    return p->height;
}

std::unique_ptr<File> Image::create_file(const std::string &name) const
{
    std::unique_ptr<File> output_file(new File);
    output_file->name = name;
    output_file->change_extension("png");

    png_structp png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    assert(png_ptr != nullptr);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert(info_ptr != nullptr);

    unsigned long bpp;
    unsigned long color_type;
    int transformations = 0;
    switch (p->fmt)
    {
        case PixelFormat::RGB:
            bpp = 3;
            color_type = PNG_COLOR_TYPE_RGB;
            break;

        case PixelFormat::BGR:
            bpp = 3;
            color_type = PNG_COLOR_TYPE_RGB;
            transformations |= PNG_TRANSFORM_BGR;
            break;

        case PixelFormat::RGBA:
            bpp = 4;
            color_type = PNG_COLOR_TYPE_RGBA;
            break;

        case PixelFormat::BGRA:
            bpp = 4;
            color_type = PNG_COLOR_TYPE_RGBA;
            transformations |= PNG_TRANSFORM_BGR;
            break;

        case PixelFormat::Grayscale:
            bpp = 1;
            color_type = PNG_COLOR_TYPE_GRAY;
            break;

        default:
            throw std::runtime_error("Invalid pixel format");
    }

    png_set_IHDR(
        png_ptr,
        info_ptr,
        p->width,
        p->height,
        8,
        color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE);

    // 0 = no compression, 9 = max compression
    // 1 produces good file size and is still fast.
    png_set_compression_level(png_ptr, 1);

    png_set_write_fn(png_ptr, &output_file->io, &png_write_data, &png_flush);
    png_write_info(png_ptr, info_ptr);

    std::unique_ptr<png_bytep[]> rows(new png_bytep[p->height]);
    for (size_t y = 0; y < p->height; y++)
    {
        size_t i = y * p->width * bpp;
        rows.get()[y] = reinterpret_cast<png_bytep>(&p->data[i]);
    }
    png_set_rows(png_ptr, info_ptr, rows.get());
    png_write_png(png_ptr, info_ptr, transformations, nullptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    return output_file;
}

u32 Image::color_at(size_t x, size_t y) const
{
    u8 r, g, b, a;
    size_t i = y * p->width + x;

    const u8 *data = reinterpret_cast<const u8*>(p->data.data());
    switch (p->fmt)
    {
        case PixelFormat::Grayscale:
            r = g = b = data[i];
            a = 255;
            break;

        case PixelFormat::RGB:
            r = data[i * 3];
            g = data[i * 3 + 1];
            b = data[i * 3 + 2];
            a = 255;
            break;

        case PixelFormat::RGBA:
            r = data[i * 4];
            g = data[i * 4 + 1];
            b = data[i * 4 + 2];
            a = data[i * 4 + 3];
            break;

        default:
            throw std::runtime_error("Unsupported pixel format");
    }
    return r | (g << 8) | (b << 16) | (a << 24);
}
