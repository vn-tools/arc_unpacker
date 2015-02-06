#include <cassert>
#include <cstring>
#include <png.h>
#include "formats/image.h"
#include "logger.h"

struct Image
{
    size_t image_width;
    size_t image_height;
    const char *pixel_data;
    char *internal_data;
    size_t pixel_data_size;
    PixelFormat pixel_format;
};

namespace
{
    void my_png_write_data(
        png_structp png_ptr,
        png_bytep data,
        png_size_t length)
    {
        IO *io = (IO*)png_get_io_ptr(png_ptr);
        if (!io_write_string(io, (char*)data, length))
            assert(0);
    }

    void my_png_read_data(
        png_structp png_ptr,
        png_bytep data,
        png_size_t length)
    {
        IO *io = (IO*)png_get_io_ptr(png_ptr);
        if (!io_read_string(io, (char*)data, length))
            assert(0);
    }

    void my_png_flush(png_structp png_ptr __attribute__((unused)))
    {
    }
}

Image *image_create_from_pixels(
    size_t image_width,
    size_t image_height,
    const char *pixel_data,
    size_t pixel_data_size,
    PixelFormat pixel_format)
{
    assert(pixel_data != nullptr);
    Image *image = new Image;
    assert(image != nullptr);
    image->image_width = image_width;
    image->image_height = image_height;
    image->pixel_data = pixel_data;
    image->pixel_data_size = pixel_data_size;
    image->pixel_format = pixel_format;
    image->internal_data = nullptr;
    return image;
}

Image *image_create_from_boxed(IO *io)
{
    assert(io != nullptr);
    io_seek(io, 0);

    Image *image = new Image;
    assert(image != nullptr);

    png_structp png_ptr = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    assert(png_ptr != nullptr);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert(info_ptr != nullptr);

    png_set_read_fn(png_ptr, io, &my_png_read_data);
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
    assert(8 == bits_per_channel);
    image->image_width = png_image_width;
    image->image_height = png_image_height;

    int bpp = 0;
    if (color_type == PNG_COLOR_TYPE_RGB)
    {
        bpp = 3;
        image->pixel_format = IMAGE_PIXEL_FORMAT_RGB;
    }
    else if (color_type == PNG_COLOR_TYPE_RGBA)
    {
        bpp = 4;
        image->pixel_format = IMAGE_PIXEL_FORMAT_RGBA;
    }
    else if (color_type == PNG_COLOR_TYPE_GRAY)
    {
        bpp = 1;
        image->pixel_format = IMAGE_PIXEL_FORMAT_GRAYSCALE;
    }
    else
    {
        assert(0);
    }

    image->pixel_data_size = image->image_width * image->image_height * bpp;
    image->internal_data = new char[image->pixel_data_size];
    assert(image->internal_data != nullptr);
    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
    size_t scanline_size = image->image_width * bpp, y;
    for (y = 0; y < image->image_height; y ++)
    {
        memcpy(
            &image->internal_data[y * scanline_size],
            row_pointers[y],
            scanline_size);
    }

    image->pixel_data = image->internal_data;
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return image;
}

void image_destroy(Image *image)
{
    assert(image != nullptr);
    delete []image->internal_data;
    delete image;
}

size_t image_width(const Image *image)
{
    assert(image != nullptr);
    return image->image_width;
}

size_t image_height(const Image *image)
{
    assert(image != nullptr);
    return image->image_height;
}

PixelFormat image_pixel_format(const Image *image)
{
    assert(image != nullptr);
    return image->pixel_format;
}

const char *image_pixel_data(const Image *image)
{
    assert(image != nullptr);
    return image->pixel_data;
}

size_t image_pixel_data_size(const Image *image)
{
    assert(image != nullptr);
    return image->pixel_data_size;
}

void image_update_file(const Image *image, VirtualFile *file)
{
    assert(image != nullptr);
    assert(file != nullptr);

    if (!io_truncate(file->io, 0))
        assert(0);

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
    switch (image->pixel_format)
    {
        case IMAGE_PIXEL_FORMAT_RGB:
            bpp = 3;
            color_type = PNG_COLOR_TYPE_RGB;
            break;

        case IMAGE_PIXEL_FORMAT_BGR:
            bpp = 3;
            color_type = PNG_COLOR_TYPE_RGB;
            transformations |= PNG_TRANSFORM_BGR;
            break;

        case IMAGE_PIXEL_FORMAT_RGBA:
            bpp = 4;
            color_type = PNG_COLOR_TYPE_RGBA;
            break;

        case IMAGE_PIXEL_FORMAT_BGRA:
            bpp = 4;
            color_type = PNG_COLOR_TYPE_RGBA;
            transformations |= PNG_TRANSFORM_BGR;
            break;

        case IMAGE_PIXEL_FORMAT_GRAYSCALE:
            bpp = 1;
            color_type = PNG_COLOR_TYPE_GRAY;
            break;

        default:
            assert(0);
            return;
    }

    png_set_IHDR(
        png_ptr,
        info_ptr,
        image->image_width,
        image->image_height,
        8,
        color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE);

    // 0 = no compression, 9 = max compression
    // 1 produces good file size and is still fast.
    png_set_compression_level(png_ptr, 1);

    png_set_write_fn(png_ptr, file->io, &my_png_write_data, &my_png_flush);
    png_write_info(png_ptr, info_ptr);

    png_bytep *rows = new png_bytep[image->image_height];
    assert(rows != nullptr);
    for (size_t y = 0; y < image->image_height; y ++)
        rows[y] = (png_bytep)&image->pixel_data[y * image->image_width * bpp];
    png_set_rows(png_ptr, info_ptr, rows);
    png_write_png(png_ptr, info_ptr, transformations, nullptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    delete []rows;

    virtual_file_change_extension(file, "png");
}

uint32_t image_color_at(const Image *image, size_t x, size_t y)
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
    size_t width = image_width(image);

    switch (image_pixel_format(image))
    {
        case IMAGE_PIXEL_FORMAT_GRAYSCALE:
            r = g = b = image_pixel_data(image)[y * width + x];
            a = 255;
            break;

        case IMAGE_PIXEL_FORMAT_RGB:
            r = image_pixel_data(image)[(y * width + x) * 3 + 0];
            g = image_pixel_data(image)[(y * width + x) * 3 + 1];
            b = image_pixel_data(image)[(y * width + x) * 3 + 2];
            a = 255;
            break;

        case IMAGE_PIXEL_FORMAT_RGBA:
            r = image_pixel_data(image)[(y * width + x) * 4 + 0];
            g = image_pixel_data(image)[(y * width + x) * 4 + 1];
            b = image_pixel_data(image)[(y * width + x) * 4 + 2];
            a = image_pixel_data(image)[(y * width + x) * 4 + 3];
            break;

        default:
            assert(0);
            return 0;
    }

    return r | (g << 8) | (b << 16) | (a << 24);
}
