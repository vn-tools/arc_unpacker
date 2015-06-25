#ifndef UTIL_IMAGE_H
#define UTIL_IMAGE_H
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

    static std::unique_ptr<Image> from_boxed(IO &io);
    static std::unique_ptr<Image> from_pixels(
        size_t width, size_t height, const std::string &data, PixelFormat fmt);

    std::unique_ptr<File> create_file(const std::string &name) const;

    size_t width() const;
    size_t height() const;
    u32 color_at(size_t x, size_t y) const;

private:
    Image();
    struct Priv;
    std::unique_ptr<Priv> p;
};

#endif
