#ifndef FORMATS_IMAGE_H
#define FORMATS_IMAGE_H
#include <stdbool.h>
#include "virtual_file.h"

typedef enum
{
    IMAGE_PIXEL_FORMAT_RGB = 1,
    IMAGE_PIXEL_FORMAT_RGBA,
    IMAGE_PIXEL_FORMAT_GRAYSCALE,
    IMAGE_PIXEL_FORMAT_BGR,
    IMAGE_PIXEL_FORMAT_BGRA
} PixelFormat;

typedef struct Image Image;

Image *image_create_from_pixels(
    size_t image_width,
    size_t image_height,
    const char *pixel_data,
    size_t pixel_data_size,
    PixelFormat pixel_format);

Image *image_create_from_boxed(IO *io);

size_t image_width(const Image *image);
size_t image_height(const Image *image);
PixelFormat image_pixel_format(const Image *image);
const char *image_pixel_data(const Image *image);
size_t image_pixel_data_size(const Image *image);
uint32_t image_color_at(const Image *image, size_t x, size_t y);

void image_destroy(Image *image);

void image_update_file(const Image *image, VirtualFile &file);

#endif
