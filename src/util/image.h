#ifndef AU_UTIL_IMAGE_H
#define AU_UTIL_IMAGE_H
#include <memory>
#include <string>
#include "io/io.h"
#include "file.h"
#include "bstr.h"

namespace au {
namespace util {

    enum PixelFormat
    {
        RGB = 1,
        RGBA,
        Grayscale,
        BGR,
        BGRA
    };

    class Image final
    {
    public:
        ~Image();

        static std::unique_ptr<Image> from_boxed(const bstr &data);
        static std::unique_ptr<Image> from_boxed(io::IO &io);
        static std::unique_ptr<Image> from_pixels(
            size_t width,
            size_t height,
            const bstr &data,
            PixelFormat fmt);

        std::unique_ptr<File> create_file(const std::string &name) const;

        size_t width() const;
        size_t height() const;
        u32 color_at(size_t x, size_t y) const;

    private:
        Image();
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }

#endif
