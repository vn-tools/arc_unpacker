#pragma once

#include <memory>
#include "io/stream.h"
#include "pix/pixel_format.h"
#include "pix/pixel.h"

namespace au {
namespace pix {

    class Palette final
    {
    public:
        Palette(const Palette &other);

        Palette(const size_t color_count);

        Palette(
            const size_t color_count,
            const bstr &input,
            const PixelFormat format);

        Palette(
            const size_t color_count,
            io::Stream &input_stream,
            const PixelFormat format);

        ~Palette();

        size_t size() const;
        Pixel &at(const size_t i);
        const Pixel &at(const size_t i) const;
        Pixel &operator [](const size_t i);
        const Pixel &operator [](const size_t i) const;

        Pixel *begin();
        Pixel *end();
        const Pixel *begin() const;
        const Pixel *end() const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
