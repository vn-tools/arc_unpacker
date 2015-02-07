#ifndef FORMATS_IMAGE_H
#define FORMATS_IMAGE_H
#include <memory>
#include <string>
#include "virtual_file.h"

typedef enum
{
    IMAGE_PIXEL_FORMAT_RGB = 1,
    IMAGE_PIXEL_FORMAT_RGBA,
    IMAGE_PIXEL_FORMAT_GRAYSCALE,
    IMAGE_PIXEL_FORMAT_BGR,
    IMAGE_PIXEL_FORMAT_BGRA
} PixelFormat;

class Image final
{
public:
    ~Image();

    static std::unique_ptr<Image> from_pixels(
        size_t image_width,
        size_t image_height,
        const std::string &pixel_data,
        PixelFormat pixel_format);

    static std::unique_ptr<Image> from_boxed(IO &io);

    void update_file(VirtualFile &target_file) const;

    size_t width() const;
    size_t height() const;
    uint32_t color_at(size_t x, size_t y) const;

private:
    Image();
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
