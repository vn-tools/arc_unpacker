#pragma once

#include <memory>
#include "io/stream.h"
#include "pix/format.h"
#include "pix/palette.h"
#include "pix/pixel.h"

namespace au {
namespace pix {

    class Image final
    {
    public:
        Image(const Image &other);

        Image(const size_t width, const size_t height);

        Image(
            const size_t width,
            const size_t height,
            const bstr &input,
            Format fmt);

        Image(
            const size_t width,
            const size_t height,
            io::Stream &input_stream,
            Format fmt);

        Image(
            const size_t width,
            const size_t height,
            const bstr &input,
            const Palette &palette);

        Image(
            const size_t width,
            const size_t height,
            io::Stream &input_stream,
            const Palette &palette);

        ~Image();

        size_t width() const;
        size_t height() const;
        Pixel &at(const size_t x, const size_t y);
        const Pixel &at(const size_t x, const size_t y) const;

        void flip_vertically();
        void flip_horizontally();
        void crop(const size_t width, const size_t height);
        void apply_mask(const Image &other);
        void apply_palette(const Palette &palette);
        void paste(const Image &other, const int target_x, const int target_y);

        Pixel *begin();
        Pixel *end();
        const Pixel *begin() const;
        const Pixel *end() const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
