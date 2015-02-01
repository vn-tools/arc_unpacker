#include <png.h>
#include <stdlib.h>
#include "assert_ex.h"
#include "image.h"
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

/* structure to store PNG image bytes */
struct PngHolder
{
    char *buffer;
    size_t size;
};

static void my_png_write_data(
    png_structp png_ptr,
    png_bytep data,
    png_size_t length)
{
    struct PngHolder *p = (struct PngHolder*) png_get_io_ptr(png_ptr);
    size_t new_size = p->size + length;

    p->buffer = p->buffer != NULL
        ? (char*)realloc(p->buffer, new_size)
        : (char*)malloc(new_size);
    assert_not_null(p->buffer);

    memcpy(p->buffer + p->size, data, length);
    p->size += length;
}

static void my_png_read_data(
    png_structp png_ptr,
    png_bytep data,
    png_size_t length)
{
    struct PngHolder *p = (struct PngHolder*) png_get_io_ptr(png_ptr);
    memcpy(data, p->buffer, length);
    p->buffer += length;
}

static void my_png_flush(png_structp png_ptr __attribute__((unused)))
{
}



Image *image_create_from_pixels(
    size_t image_width,
    size_t image_height,
    const char *pixel_data,
    size_t pixel_data_size,
    PixelFormat pixel_format)
{
    Image *image = (Image*)malloc(sizeof(Image));
    image->image_width = image_width;
    image->image_height = image_height;
    image->pixel_data = pixel_data;
    image->pixel_data_size = pixel_data_size;
    image->pixel_format = pixel_format;
    image->internal_data = NULL;
    return image;
}

Image *image_create_from_boxed(const char *boxed_data, size_t boxed_data_size)
{
    assert_not_null(boxed_data);

    Image *image = (Image*)malloc(sizeof(Image));

    struct PngHolder png_holder;
    png_holder.buffer = (char*)boxed_data;
    png_holder.size = boxed_data_size;

    png_structp png_ptr = png_create_read_struct(
        PNG_LIBPNG_VER_STRING,
        NULL,
        NULL,
        NULL);
    assert_not_null(png_ptr);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert_not_null(info_ptr);
    assert_that(setjmp(png_jmpbuf(png_ptr)) == 0);

    png_set_read_fn(png_ptr, &png_holder, &my_png_read_data);
    png_read_png(
        png_ptr,
        info_ptr,
        PNG_TRANSFORM_GRAY_TO_RGB
            | PNG_TRANSFORM_STRIP_16
            | PNG_TRANSFORM_PACKING
            | PNG_TRANSFORM_EXPAND,
        NULL);

    int color_type;
    int bits_per_channel;
    png_get_IHDR(
        png_ptr,
        info_ptr,
        &image->image_width,
        &image->image_height,
        &bits_per_channel,
        &color_type,
        NULL,
        NULL,
        NULL);
    assert_equali(8, bits_per_channel);

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
        log_error("Unsupported color_type: %d\n", color_type);
        assert_that(1 == 0);
    }

    image->pixel_data_size = image->image_width * image->image_height * bpp;
    image->internal_data = (char*)malloc(image->pixel_data_size);
    assert_not_null(image->internal_data);
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

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    return image;
}

void image_destroy(Image *image)
{
    assert_not_null(image);
    free(image->internal_data);
    free(image);
}

size_t image_width(const Image *image)
{
    assert_not_null(image);
    return image->image_width;
}

size_t image_height(const Image *image)
{
    assert_not_null(image);
    return image->image_height;
}

PixelFormat image_pixel_format(const Image *image)
{
    assert_not_null(image);
    return image->pixel_format;
}

const char *image_pixel_data(const Image *image)
{
    assert_not_null(image);
    return image->pixel_data;
}

size_t image_pixel_data_size(const Image *image)
{
    assert_not_null(image);
    return image->pixel_data_size;
}

void image_update_file(const Image *image, VirtualFile *file)
{
    assert_not_null(image);
    assert_not_null(file);

    struct PngHolder png_holder;
    png_holder.buffer = NULL;
    png_holder.size = 0;

    png_structp png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING,
        NULL,
        NULL,
        NULL);
    assert_not_null(png_ptr);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert_not_null(info_ptr);
    assert_that(setjmp(png_jmpbuf(png_ptr)) == 0);

    unsigned long bpp;
    unsigned long color_type;
    int transformations = 0;
    if (image->pixel_format == IMAGE_PIXEL_FORMAT_RGB)
    {
        bpp = 3;
        color_type = PNG_COLOR_TYPE_RGB;
    }
    else if (image->pixel_format == IMAGE_PIXEL_FORMAT_BGR)
    {
        bpp = 3;
        color_type = PNG_COLOR_TYPE_RGB;
        transformations |= PNG_TRANSFORM_BGR;
    }
    else if (image->pixel_format == IMAGE_PIXEL_FORMAT_RGBA)
    {
        bpp = 4;
        color_type = PNG_COLOR_TYPE_RGBA;
    }
    else if (image->pixel_format == IMAGE_PIXEL_FORMAT_BGRA)
    {
        bpp = 4;
        color_type = PNG_COLOR_TYPE_RGBA;
        transformations |= PNG_TRANSFORM_BGR;
    }
    else if (image->pixel_format == IMAGE_PIXEL_FORMAT_GRAYSCALE)
    {
        bpp = 1;
        color_type = PNG_COLOR_TYPE_GRAY;
    }
    else
    {
        log_error("Wrong color type");
        assert_that(1 == 0);
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

    png_set_write_fn(png_ptr, &png_holder, &my_png_write_data, &my_png_flush);
    png_write_info(png_ptr, info_ptr);

    png_bytep* rows = (png_bytep*)malloc(sizeof(png_bytep)*image->image_height);
    assert_not_null(rows);
    size_t y;
    for (y = 0; y < image->image_height; y ++)
        rows[y] = (png_bytep)&image->pixel_data[y * image->image_width * bpp];
    png_set_rows(png_ptr, info_ptr, rows);
    png_write_png(png_ptr, info_ptr, transformations, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    free(rows);

    vf_change_extension(file, "png");
    vf_set_data(file, png_holder.buffer, png_holder.size);
    free(png_holder.buffer);
}
