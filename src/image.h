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

size_t image_width(Image *image);
size_t image_height(Image *image);
PixelFormat image_pixel_format(Image *image);
const char *image_pixel_data(Image *image);
size_t image_pixel_data_size(Image *image);

void image_destroy(Image *image);

void image_update_file(Image *image, VirtualFile *file);
