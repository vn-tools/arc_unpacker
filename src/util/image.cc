#include <cassert>
#include <png.h>
#include "util/image.h"

struct Image::Internals
{
    size_t image_width;
    size_t image_height;
    std::string pixel_data;
    size_t pixel_data_size;
    PixelFormat pixel_format;
};

namespace
{
    void my_png_write_data(
        png_structp png_ptr, png_bytep data, png_size_t length)
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

    void my_png_flush(png_structp)
    {
    }
}

Image::Image() : internals(new Internals)
{
}

Image::~Image()
{
}

std::unique_ptr<Image> Image::from_pixels(
    size_t image_width,
    size_t image_height,
    const std::string &pixel_data,
    PixelFormat pixel_format)
{
    std::unique_ptr<Image> image(new Image);
    if (image_width == 0 || image_height == 0)
        throw std::runtime_error("Image dimension cannot be 0");
    image->internals->image_width = image_width;
    image->internals->image_height = image_height;
    image->internals->pixel_data = pixel_data;
    image->internals->pixel_format = pixel_format;
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
    png_uint_32 png_image_width, png_image_height;
    png_get_IHDR(
        png_ptr,
        info_ptr,
        &png_image_width,
        &png_image_height,
        &bits_per_channel,
        &color_type,
        nullptr,
        nullptr,
        nullptr);
    assert(bits_per_channel == 8);

    std::unique_ptr<Image> image(new Image);
    image->internals->image_width = png_image_width;
    image->internals->image_height = png_image_height;

    int bpp = 0;
    switch (color_type)
    {
        case PNG_COLOR_TYPE_RGB:
            bpp = 3;
            image->internals->pixel_format = PixelFormat::RGB;
            break;
        case PNG_COLOR_TYPE_RGBA:
            bpp = 4;
            image->internals->pixel_format = PixelFormat::RGBA;
            break;
        case PNG_COLOR_TYPE_GRAY:
            bpp = 1;
            image->internals->pixel_format = PixelFormat::Grayscale;
            break;
        default:
            throw std::runtime_error("Bad pixel format");
    }

    image->internals->pixel_data_size
        = png_image_width * png_image_height * bpp;
    image->internals->pixel_data
        = std::string(image->internals->pixel_data_size, '\0');

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
    size_t scanline_size = png_image_width * bpp, y;
    for (y = 0; y < png_image_height; y ++)
    {
        image->internals->pixel_data.replace(
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
    return internals->image_width;
}

size_t Image::height() const
{
    return internals->image_height;
}

std::unique_ptr<File> Image::create_file(const std::string &name) const
{
    std::unique_ptr<File> output_file(new File);
    output_file->name = name;
    output_file->change_extension("png");

    png_structp png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING,
        nullptr,
        nullptr,
        nullptr);
    assert(png_ptr != nullptr);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert(info_ptr != nullptr);

    unsigned long bpp;
    unsigned long color_type;
    int transformations = 0;
    switch (internals->pixel_format)
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
        internals->image_width,
        internals->image_height,
        8,
        color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE);

    // 0 = no compression, 9 = max compression
    // 1 produces good file size and is still fast.
    png_set_compression_level(png_ptr, 1);

    png_set_write_fn(
        png_ptr, &output_file->io, &my_png_write_data, &my_png_flush);
    png_write_info(png_ptr, info_ptr);

    std::unique_ptr<png_bytep[]> rows(new png_bytep[internals->image_height]);
    for (size_t y = 0; y < internals->image_height; y ++)
    {
        rows.get()[y]
            = reinterpret_cast<png_bytep>(&internals->pixel_data
                [y * internals->image_width * bpp]);
    }
    png_set_rows(png_ptr, info_ptr, rows.get());
    png_write_png(png_ptr, info_ptr, transformations, nullptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    return output_file;
}

uint32_t Image::color_at(size_t x, size_t y) const
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
    size_t i = y * internals->image_width + x;

    const uint8_t *pixel_data
        = reinterpret_cast<const uint8_t*>(internals->pixel_data.data());

    switch (internals->pixel_format)
    {
        case PixelFormat::Grayscale:
            r = g = b = pixel_data[i];
            a = 255;
            break;

        case PixelFormat::RGB:
            r = pixel_data[i * 3];
            g = pixel_data[i * 3 + 1];
            b = pixel_data[i * 3 + 2];
            a = 255;
            break;

        case PixelFormat::RGBA:
            r = pixel_data[i * 4];
            g = pixel_data[i * 4 + 1];
            b = pixel_data[i * 4 + 2];
            a = pixel_data[i * 4 + 3];
            break;

        default:
            throw std::runtime_error("Unsupported pixel format");
    }

    return r | (g << 8) | (b << 16) | (a << 24);
}
