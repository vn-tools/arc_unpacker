#ifndef AU_UTIL_IMAGE_H
#define AU_UTIL_IMAGE_H
#include <memory>
#include <vector>
#include <string>
#include <type_traits>
#include "file.h"
#include "io/io.h"
#include "types.h"

namespace au {
namespace util {

    struct Color
    {
        u8 b;
        u8 g;
        u8 r;
        u8 a;

        constexpr bool operator ==(const Color &other)
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }

        constexpr bool operator !=(const Color &other)
        {
            return !(operator ==(other));
        }

        constexpr const u8 &operator[](size_t x) const
        {
            return reinterpret_cast<const u8*>(this)[x];
        }

        inline u8 &operator[](size_t x)
        {
            return reinterpret_cast<u8*>(this)[x];
        }
    };

    static_assert(sizeof(Color) == 4, "!");
    static_assert(std::is_pod<Color>::value, "!");

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
            size_t width, size_t height, const bstr &data, PixelFormat fmt);
        static std::unique_ptr<Image> from_pixels(
            size_t width, size_t height, const std::vector<Color> &data);

        std::unique_ptr<File> create_file(const std::string &name) const;

        size_t width() const;
        size_t height() const;
        Color color_at(size_t x, size_t y) const;

    private:
        Image();
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }

#endif
