#include <stdbool.h>
#include "virtual_file.h"

typedef enum
{
    IMAGE_PIXEL_FORMAT_RGB = 1,
    IMAGE_PIXEL_FORMAT_RGBA,
} PixelFormat;

typedef struct Image Image;

Image *image_create_from_pixels(
    size_t image_width,
    size_t image_height,
    const char *pixel_data,
    size_t pixel_data_size,
    PixelFormat pixel_format);

Image *image_create_from_boxed(const char *boxed_data, size_t boxed_data_size);

size_t image_width(const Image *image);
size_t image_height(const Image *image);
PixelFormat image_pixel_format(const Image *image);
const char *image_pixel_data(const Image *image);
size_t image_pixel_data_size(const Image *image);

void image_destroy(Image *image);

void image_update_file(const Image *image, VirtualFile *file);
