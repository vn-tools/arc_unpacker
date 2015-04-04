#ifndef FORMATS_IMAGE_H
#define FORMATS_IMAGE_H
#include <memory>
#include <string>
#include "file.h"

typedef enum
{
    RGB = 1,
    RGBA,
    Grayscale,
    BGR,
    BGRA
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

    std::unique_ptr<File> create_file(const std::string &name) const;

    size_t width() const;
    size_t height() const;
    uint32_t color_at(size_t x, size_t y) const;

private:
    Image();
    struct Internals;
    std::unique_ptr<Internals> internals;
};

#endif
